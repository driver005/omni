module;
export module app_handlers:app;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_store;
import app_http;
import app_crypto;
import app_metrics;
import app_services;
import app_events;
import app_workflow;
import app_keystore;

export namespace app::handlers::app {

/**
 * @brief Synthetic /app surface mirroring the internal AppState/SessionState helpers in
 * upstream `crates/router/src/routes/app.rs`. Upstream defines no /app HTTP scope; these five
 * helpers are startup/internal constructors with no HTTP auth. RouteAuthRegistry therefore
 * resolves every operation below to a NO_AUTH rule (evidence app.rs:207-212, 477-495,
 * 497-643, 648-727, 729-743), and authorize_route is a pass-through. Real per-scope auth
 * lives in the other domain handlers and is tracked in their own parity reports.
 */
class AppRouteHandlers
{
public:
    explicit AppRouteHandlers(::app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_payment_callback_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream SessionState::payment_callback_config() is an internal OnceCell load
        // (app.rs:207-212) with no HTTP auth. The registry row is NO_AUTH.
        const auto rule = m_context->auth_rules().rule_for("payment_callback_config");
        const auto auth = m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        // Load the runtime rollout policy stored under the "payment_callback" config key.
        // ConfigStore (superposition-backed) is consulted first, then the repository config
        // and routing-config mirrors; the cache is only the session-local OnceCell stand-in.
        std::string config_source;
        const auto stored = load_payment_callback_config(config_source);
        if (!stored) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND, "payment_callback config is missing"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", *stored);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_JSON, "invalid payment_callback config"));
            return;
        }

        const auto mode = field_string(*parsed, "mode").value_or("");
        if (mode != "legacy" && mode != "migration" && mode != "enforced") {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "payment_callback mode must be legacy, migration or enforced"));
            return;
        }
        // Mirrors PaymentCallbackMode::issues_tokens()/allows_legacy().
        const bool issues_tokens = mode != "legacy";
        const bool allows_legacy = mode != "enforced";

        m_context->metrics().record_flow("PaymentCallbackConfigLoad");
        m_context->metrics().counter("PAYMENT_CALLBACK_CONFIG_LOADED").increment();

        const auto text = std::string{R"({"partition":"app","op":"payment_callback_config","mode":)"}
            + serde::Ser::encode_json(mode)
            + R"(,"issues_tokens":)" + (issues_tokens ? "true" : "false")
            + R"(,"allows_legacy":)" + (allows_legacy ? "true" : "false")
            + R"(,"config_source":)" + serde::Ser::encode_json(config_source)
            + "}";
        send_json(res, send, text, true);
    }

    void handle_create_email_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream create_email_client() is a free helper (app.rs:477-495), no HTTP auth.
        const auto rule = m_context->auth_rules().rule_for("create_email_client");
        const auto auth = m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream create_email_client(): choose from EmailClientConfigs
        // (ses / smtp / no_email_client). Credentials are secrets and must not
        // be returned to the caller in the clear.
        const auto configured_client = field_string(*parsed, "client_config");
        auto client_type = configured_client.value_or("no_email_client");
        if (client_type != "ses" && client_type != "smtp" && client_type != "no_email_client") {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "email client_config must be ses, smtp or no_email_client"));
            return;
        }
        // The upstream config-driven default when the caller omits client_config.
        if (!configured_client) {
            client_type = m_context->config_store()
                              .get_value("email_client", "default_config")
                              .value_or(client_type);
        }

        const auto query = ::app::http::RequestQuery{}(req);
        const auto proxy_url = query.get("proxy_url").value_or(
            m_context->config_store().get_value("email_client", "proxy_url").value_or(""));
        const auto client_id = m_context->ids().key_id();

        // Deterministic substitute for upstream secret fetch (fetch_raw_secrets): a caller may
        // pass a key-store id; the active value is resolved through the injected KeyStore and
        // never echoed. The literal field is only a fallback for callers without a managed key.
        ::app::crypto::SecretString secret{field_string(*parsed, "secret").value_or("")};
        bool key_configured{false};
        const auto secret_id = field_string(*parsed, "secret_id").value_or("");
        if (!secret_id.empty()) {
            const auto managed = m_context->key_store().get("email_client", secret_id);
            if (managed) {
                secret = *managed;
                key_configured = true;
            }
        }

        // Ephemeral registry of freshly constructed clients, keyed by instance id. Only the
        // selection is cached; the raw request body and the secret are never cached.
        m_context->cache().put(
            "app.email_client:" + client_id,
            core::cache::to_cache_value(client_type)
        );

        // Event-side substitute for constructing the upstream Box<dyn EmailService>: record the
        // deterministic construction as a persisted event instead of returning a fake client.
        const auto event_id = m_context->events().publish(
            "email_client.created",
            std::string{R"({"client_id":)"} + serde::Ser::encode_json(client_id)
                + R"(,"client_config":)" + serde::Ser::encode_json(client_type) + "}"
        );

        m_context->metrics().record_flow("CreateEmailClient");
        m_context->metrics().counter("EMAIL_CLIENT_CREATED").increment();

        const auto text = std::string{R"({"partition":"app","op":"create_email_client","client_id":)"}
            + serde::Ser::encode_json(client_id)
            + R"(,"client_config":)" + serde::Ser::encode_json(client_type)
            + R"(,"proxy_url":)" + serde::Ser::encode_json(proxy_url)
            + R"(,"secret":)" + serde::Ser::encode_json(
                secret.masked_as(::app::crypto::SecretFieldKind::ConnectorCredential))
            + R"(,"key_configured":)" + (key_configured ? "true" : "false")
            + R"(,"event_id":)" + serde::Ser::encode_json(event_id)
            + "}";
        send_json(res, send, text, true);
    }

    void handle_with_storage(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream AppState::with_storage() is an async constructor (app.rs:497-643), no auth.
        const auto rule = m_context->auth_rules().rule_for("with_storage");
        const auto auth = m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        // The synthetic HTTP surface reports the chosen storage backend and probes the store.
        const auto query = ::app::http::RequestQuery{}(req);
        const auto storage_impl = query.get("storage_impl").value_or("postgresql");
        const auto service_name = query.get("service_name").value_or("default");
        if (!known_storage_impl(storage_impl)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "storage_impl must be postgresql, postgresql_test or mock"));
            return;
        }

        const bool storage_ok = storage_available();
        const auto session_id = m_context->ids().uuid_v4();

        // Deterministic substitutes for the upstream state-assembly dependencies:
        //  * secrets_configured stands in for fetch_raw_secrets / the secret-management client.
        //  * superposition_backend resolves the storage choice through the superposition client.
        //  * process_tracker_id records the async constructor as a scheduled tracker.
        const bool secrets_configured =
            m_context->secret_resolver().resolve("jwt", "jwt_secret").has_value();
        const auto resolved = m_context->superposition().resolve("app_state", "storage_impl", service_name);
        const auto superposition_backend = resolved ? resolved->get_value() : storage_impl;
        const auto payload = std::string{R"({"session_id":)"} + serde::Ser::encode_json(session_id)
            + R"(,"storage_impl":)" + serde::Ser::encode_json(storage_impl) + "}";
        const auto process_tracker_id =
            m_context->process_trackers().schedule("app_state_with_storage", payload, "0");

        m_context->cache().put(
            "app.with_storage:" + session_id,
            core::cache::to_cache_value(storage_impl)
        );

        m_context->metrics().record_flow("AppStateWithStorage");
        m_context->metrics().counter("APP_STATE_WITH_STORAGE").increment();

        const auto text = std::string{R"({"partition":"app","op":"with_storage","session_id":)"}
            + serde::Ser::encode_json(session_id)
            + R"(,"storage_impl":)" + serde::Ser::encode_json(storage_impl)
            + R"(,"service_name":)" + serde::Ser::encode_json(service_name)
            + R"(,"storage_ok":)" + (storage_ok ? "true" : "false")
            + R"(,"secrets_configured":)" + (secrets_configured ? "true" : "false")
            + R"(,"superposition_backend":)" + serde::Ser::encode_json(superposition_backend)
            + R"(,"process_tracker_id":)" + std::to_string(process_tracker_id)
            + "}";
        send_json(res, send, text, true);
    }

    void handle_get_store_interface(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream AppState::get_store_interface() is an async constructor (app.rs:648-727).
        const auto rule = m_context->auth_rules().rule_for("get_store_interface");
        const auto auth = m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        // Pick a storage interface from (storage_impl, event_handler) for the caller's tenant.
        const auto query = ::app::http::RequestQuery{}(req);
        const auto storage_impl = query.get("storage_impl").value_or("postgresql");
        const auto event_handler = query.get("event_handler").value_or("logs");
        if (!known_storage_impl(storage_impl)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "storage_impl must be postgresql, postgresql_test or mock"));
            return;
        }
        if (event_handler != "kafka" && event_handler != "logs") {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "event_handler must be kafka or logs"));
            return;
        }

        const std::string interface_name =
            storage_impl == "mock" ? "mock_db"
                                   : (event_handler == "kafka" ? "kafka_store" : "postgres_store");
        const auto org_id = ::app::http::org_id_from_header{}(req);
        const auto merchant_id = ::app::http::merchant_id_from_header{}(req);
        bool tenant_known = true;
        if (!merchant_id.empty()) {
            try {
                tenant_known = m_context->repository()
                                   .find_merchant_account_by_string(merchant_id)
                                   .has_value();
            } catch (...) {
                tenant_known = false;
            }
        }

        // Deterministic substitutes for upstream startup state that has no HTTP wire shape:
        //  * KeyManagerState construction -> active master key presence in the KeyStore.
        //  * mTLS cert/CA fields -> presence of resolved secrets, never their raw value.
        //  * KafkaStore/Logs event handler -> a persisted event record.
        const bool key_manager_configured =
            m_context->key_store().get("key_manager", "master_key").has_value();
        const bool mtls_cert_configured =
            m_context->secret_resolver().resolve("key_manager", "mtls_cert").has_value();
        const bool mtls_ca_configured =
            m_context->secret_resolver().resolve("key_manager", "mtls_ca").has_value();
        const auto event_id = m_context->events().publish(
            "store_interface.resolved",
            std::string{R"({"interface":)"} + serde::Ser::encode_json(interface_name)
                + R"(,"event_handler":)" + serde::Ser::encode_json(event_handler) + "}"
        );

        m_context->metrics().record_flow("GetStoreInterface");
        m_context->metrics().counter("STORE_INTERFACE_RESOLVED").increment();

        const auto text = std::string{R"({"partition":"app","op":"get_store_interface","interface":)"}
            + serde::Ser::encode_json(interface_name)
            + R"(,"storage_impl":)" + serde::Ser::encode_json(storage_impl)
            + R"(,"event_handler":)" + serde::Ser::encode_json(event_handler)
            + R"(,"org_id":)" + serde::Ser::encode_json(org_id)
            + R"(,"merchant_id":)" + serde::Ser::encode_json(merchant_id)
            + R"(,"tenant_known":)" + (tenant_known ? "true" : "false")
            + R"(,"key_manager_configured":)" + (key_manager_configured ? "true" : "false")
            + R"(,"mtls_cert_configured":)" + (mtls_cert_configured ? "true" : "false")
            + R"(,"mtls_ca_configured":)" + (mtls_ca_configured ? "true" : "false")
            + R"(,"event_id":)" + serde::Ser::encode_json(event_id)
            + "}";
        send_json(res, send, text, true);
    }

    void handle_new(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream AppState::new() delegates to with_storage(Postgresql) (app.rs:729-743).
        const auto rule = m_context->auth_rules().rule_for("new");
        const auto auth = m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto query = ::app::http::RequestQuery{}(req);
        const auto service_name = query.get("service_name").value_or("default");
        const auto session_id = m_context->ids().uuid_v4();
        const auto storage_impl = m_context->config_store()
                                      .get_value("app_state", "default_storage_impl")
                                      .value_or(std::string{"postgresql"});
        const auto process_tracker_id = m_context->process_trackers().schedule(
            "app_state_new",
            std::string{R"({"session_id":)"} + serde::Ser::encode_json(session_id) + "}",
            "0"
        );
        m_context->cache().put(
            "app.new:" + session_id,
            core::cache::to_cache_value(storage_impl)
        );

        const bool storage_ok = storage_available();
        m_context->metrics().record_flow("AppStateNew");
        m_context->metrics().counter("APP_STATE_NEW").increment();

        const auto text = std::string{R"({"partition":"app","op":"new","session_id":)"}
            + serde::Ser::encode_json(session_id)
            + R"(,"storage_impl":)" + serde::Ser::encode_json(storage_impl)
            + R"(,"service_name":)" + serde::Ser::encode_json(service_name)
            + R"(,"storage_ok":)" + (storage_ok ? "true" : "false")
            + R"(,"process_tracker_id":)" + std::to_string(process_tracker_id)
            + "}";
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/app"}
            .add_route(
                Route{"/payment_callback_config"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_callback_config(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_callback_config")
            );
        Router{router, "/app"}
            .add_route(
                Route{"/create_email_client"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_email_client(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_email_client")
            );
        Router{router, "/app"}
            .add_route(
                Route{"/with_storage"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_with_storage(req, res, std::move(send));
                    })
                    .summary("Hyperswitch with_storage")
            );
        Router{router, "/app"}
            .add_route(
                Route{"/get_store_interface"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_store_interface(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_store_interface")
            );
        Router{router, "/app"}
            .add_route(
                Route{"/new"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_new(req, res, std::move(send));
                    })
                    .summary("Hyperswitch new")
            );
    }

private:
    [[nodiscard]] std::optional<std::string> field_string(
        const serde::Value& value,
        std::string_view name
    ) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return std::nullopt;
        }
        const auto field = object->get(std::string{name});
        if (!field) {
            return std::nullopt;
        }
        if (const auto* text = std::get_if<std::string>(&field.value().get())) {
            return *text;
        }
        return std::nullopt;
    }

    [[nodiscard]] bool known_storage_impl(std::string_view value) const noexcept
    {
        return value == "postgresql" || value == "postgresql_test" || value == "mock";
    }

    // PaymentCallbackConfig is a config entry (stored as JSON), not one of the typed domain
    // entities. ConfigStore is the persisted source, repository config/routing-config are the
    // compatibility mirrors, and the cache stands in for the session-local OnceCell.
    [[nodiscard]] std::optional<std::string> load_payment_callback_config(std::string& source) const
    {
        constexpr std::string_view key{"payment_callback"};
        constexpr std::string_view cache_key{"app.config.payment_callback"};
        const auto cached = m_context->cache().get(std::string{cache_key});
        if (cached.hit) {
            source = "session_cache";
            return cached.value_or<std::string>(std::string{});
        }
        try {
            const auto config_store_value = m_context->config_store().get_value("payment_callback", "config");
            if (config_store_value && !config_store_value->empty()) {
                m_context->cache().put(std::string{cache_key}, core::cache::to_cache_value(*config_store_value));
                source = "config_store";
                return config_store_value;
            }
            const auto repo_value = m_context->repository().get_config(key);
            if (repo_value && !repo_value->empty()) {
                m_context->cache().put(std::string{cache_key}, core::cache::to_cache_value(*repo_value));
                source = "repository_config";
                return repo_value;
            }
            const auto record = m_context->repository().find_routing_config_by_string(key);
            if (record && !record->get_config_json().empty()) {
                const auto text = record->get_config_json();
                m_context->cache().put(std::string{cache_key}, core::cache::to_cache_value(text));
                source = "routing_config";
                return text;
            }
        } catch (...) {
            return std::nullopt;
        }
        return std::nullopt;
    }

    [[nodiscard]] bool storage_available() const
    {
        try {
            (void)m_context->repository().list_organizations();
            return true;
        } catch (...) {
            return false;
        }
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::app
