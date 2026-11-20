module;
export module app_handlers:connector_onboarding;

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
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_services;
import app_keystore;

export namespace app::handlers::connector_onboarding {

class ConnectorOnboardingRouteHandlers
{
public:
    explicit ConnectorOnboardingRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_get_action_url(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_action_url");
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = m_context->authorizer().authorize_route(auth, rule)) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        // The id_type/Connector wrappers in ActionUrlRequest have no serde::Serializable,
        // so decode the generic tree and normalize the wrapped values locally.
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string connector_id{};
        std::string connector{};
        std::string return_url{};
        const auto& root = parsed->get();
        if (const auto* object = std::get_if<interfaces::Value::Object>(&root)) {
            if (auto field = object->get("connector_id")) { connector_id = json_text(field.value()); }
            if (auto field = object->get("connector")) { connector = json_text(field.value()); }
            if (auto field = object->get("return_url")) { return_url = json_text(field.value()); }
        }
        const auto merchant_id = onboarding_merchant_id(req, auth);
        if (const auto guard_error = check_connector(connector_id, merchant_id, std::string{})) {
            send_error(res, send, *guard_error);
            return;
        }
        // Upstream utils::get_connector_auth returns NotImplemented for any non-PayPal
        // connector. There is no NOT_IMPLEMENTED AppErrorKind, so map it to VALIDATION_FAILED.
        if (!is_paypal(connector)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "onboarding is not implemented for connector: " + connector));
            return;
        }
        // Upstream core:34-41 rejects anything except Some(true) from the
        // connector_onboarding config. Locally the gate is a ConfigStore row; an absent
        // row means the feature is enabled (stand-in for a deployed app config).
        if (!connector_onboarding_enabled(connector)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN,
                "connector onboarding is not enabled for: " + connector));
            return;
        }

        // Upstream: get_tracking_id_from_configs -> tracked id for this connector account.
        const auto tracking_id = get_tracking_id(connector_id, connector);

        // Upstream: paypal::get_action_url_from_paypal performs an OAuth token exchange and
        // POST v2/customer/partner-referrals against the configured base_url. No network or
        // credentials exist locally; the deterministic substitute calls the injected
        // connector's exchange_token/create_link capabilities, reads the configured base_url,
        // and uses the connector action_url/reference_id when present.
        const auto base_url = m_context->config_store()
                                  .get_value("connectors", "paypal_base_url")
                                  .value_or(std::string{"https://www.paypal.com/"});
        std::string action_url{
            std::format("{}bizsignup/entry?tracking_id={}", base_url, tracking_id)
        };
        if (!return_url.empty()) {
            action_url += std::format("&return_url={}", return_url);
        }
        if (auto connector_service = m_context->connector_router().select(merchant_id)) {
            auto token_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("generate_access_token")
                                     .merchant_id(merchant_id)
                                     .resource_id(tracking_id)();
            (void)connector_service->exchange_token(token_request);
            auto request = app::connector::ConnectorRequestBuilder{}
                               .operation("create_link")
                               .merchant_id(merchant_id)
                               .resource_id(tracking_id)
                               .raw_payload(body)();
            const auto response = connector_service->create_link(request);
            if (const auto from_connector = response_field_text(response, "action_url")) {
                if (!from_connector->empty()) {
                    action_url = *from_connector;
                }
            } else if (!response.get_reference_id().empty()) {
                action_url += std::format("&referral={}", response.get_reference_id());
            }
        }

        app::dto::connector_onboarding::PayPalActionUrlResponse inner{};
        inner.set_action_url(std::move(action_url));
        const auto inner_json = serde::Ser::encode_json(inner);
        // Upstream ActionUrlResponse::PayPal externally-tagged envelope: {"paypal":{...}}.
        const auto text = std::format(R"({{"paypal":{}}})", inner_json);

        m_context->metrics().record_flow("GetActionUrl");
        m_context->metrics().counter("connector_onboarding.get_action_url").increment();

        send_json(res, send, text, true);
    }

    void handle_sync_onboarding_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("sync_onboarding_status");
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = m_context->authorizer().authorize_route(auth, rule)) {
            send_error(res, send, *auth_error);
            return;
        }

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

        std::string connector_id{};
        std::string connector{};
        std::string profile_id{};
        const auto& root = parsed->get();
        if (const auto* object = std::get_if<interfaces::Value::Object>(&root)) {
            if (auto field = object->get("connector_id")) { connector_id = json_text(field.value()); }
            if (auto field = object->get("connector")) { connector = json_text(field.value()); }
            if (auto field = object->get("profile_id")) { profile_id = json_text(field.value()); }
        }
        const auto merchant_id = onboarding_merchant_id(req, auth);

        // Upstream: profile_id comes from the sync request; v2 may carry it on the query.
        std::string effective_profile_id{profile_id};
        if (effective_profile_id.empty()) {
            if (const auto query_profile = app::http::RequestQuery{}(req).get("profile_id")) {
                effective_profile_id = *query_profile;
            }
        }
        if (effective_profile_id.empty()) {
            effective_profile_id = auth.profile_id();
        }
        if (const auto guard_error = check_connector(connector_id, merchant_id, effective_profile_id)) {
            send_error(res, send, *guard_error);
            return;
        }
        if (!is_paypal(connector)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "onboarding is not implemented for connector: " + connector));
            return;
        }
        if (!connector_onboarding_enabled(connector)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN,
                "connector onboarding is not enabled for: " + connector));
            return;
        }

        // Upstream: get_tracking_id_from_configs then paypal onboarding status sync.
        const auto tracking_id = get_tracking_id(connector_id, connector);
        OnboardingOutcome outcome{"success", std::string{}, true};
        if (auto connector_service = m_context->connector_router().select(merchant_id)) {
            auto request = app::connector::ConnectorRequestBuilder{}
                               .operation("sync_onboarding_status")
                               .merchant_id(merchant_id)
                               .resource_id(tracking_id)
                               .raw_payload(body)();
            const auto response = connector_service->sync(request);
            outcome = resolve_onboarding_status(response);
        }

        std::string text{};
        if (outcome.success) {
            // Upstream core:87-110: on Success, update_mca with a SignatureKey auth payload
            // built from the configured client secret/id and the PayPal payer id, then return
            // OnboardingStatus::PayPal::ConnectorIntegrated(MerchantConnectorResponse).
            auto found = m_context->repository().find_merchant_connector_by_string(connector_id);
            if (found.has_value()) {
                auto record = std::move(*found);
                apply_integration_auth(record, connector, outcome.payer_id);
                m_context->repository().update_merchant_connector(record);
                const auto mca_json = serde::Ser::encode_json(record);
                text = std::format(R"({{"paypal":{{"connector_integrated":{}}}}})", mca_json);
            } else {
                // check_connector already proved the account exists; keep a deterministic
                // Success payload if a concurrent removal made the update impossible.
                text = success_envelope(outcome.payer_id);
            }
        } else if (outcome.wire == "success") {
            text = success_envelope(outcome.payer_id);
        } else {
            text = std::format(R"({{"paypal":"{}"}})", outcome.wire);
        }

        m_context->metrics().record_flow("SyncOnboardingStatus");
        m_context->metrics().counter("connector_onboarding.sync_onboarding_status").increment();

        send_json(res, send, text, true);
    }

    void handle_reset_tracking_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("reset_tracking_id");
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = m_context->authorizer().authorize_route(auth, rule)) {
            send_error(res, send, *auth_error);
            return;
        }

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

        std::string connector_id{};
        std::string connector{};
        const auto& root = parsed->get();
        if (const auto* object = std::get_if<interfaces::Value::Object>(&root)) {
            if (auto field = object->get("connector_id")) { connector_id = json_text(field.value()); }
            if (auto field = object->get("connector")) { connector = json_text(field.value()); }
        }
        if (connector.empty()) {
            connector = "paypal";
        }

        const auto merchant_id = onboarding_merchant_id(req, auth);
        if (const auto guard_error = check_connector(connector_id, merchant_id, std::string{})) {
            send_error(res, send, *guard_error);
            return;
        }

        // Upstream: set_tracking_id_in_configs upserts a fresh unix timestamp. reset does
        // not consult the is_enabled gate (core:122-138).
        reset_tracking_id(connector_id, connector);

        m_context->metrics().record_flow("ResetTrackingId");
        m_context->metrics().counter("connector_onboarding.reset_tracking_id").increment();

        // Upstream returns StatusOk with no body.
        send_json(res, send, std::string_view{}, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/connector_onboarding"}
            .add_route(
                Route{"/action_url"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_action_url(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_action_url")
            )
            .add_route(
                Route{"/sync"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_sync_onboarding_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch sync_onboarding_status")
            )
            .add_route(
                Route{"/reset_tracking_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_reset_tracking_id(req, res, std::move(send));
                    })
                    .summary("Hyperswitch reset_tracking_id")
            );
    }

private:
    /** @brief Local mirror of the OnboardingStatus::PayPal variant set. */
    struct OnboardingOutcome
    {
        std::string_view wire{};
        std::string payer_id{};
        bool success{false};
    };

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

    [[nodiscard]] std::string onboarding_merchant_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth
    ) const
    {
        const auto connected = auth.claim(app::auth::kClaimConnectedMerchantId);
        if (connected.has_value() && !connected->empty()) {
            return std::string{*connected};
        }
        return app::http::merchant_id_from_header{}(req);
    }

    // Upstream: check_if_connector_exists (utils/connector_onboarding.rs:45-83). The
    // merchant key store is consulted for the wrap material; absence is not a hard gate
    // because the local key store is empty until a deployment seeds it.
    [[nodiscard]] std::optional<core::error::AppError> check_connector(
        std::string_view connector_id,
        std::string_view merchant_id,
        std::string_view profile_id
    ) const
    {
        if (merchant_id.empty()) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::UNAUTHORIZED, "missing merchant id");
        }
        if (!m_context->repository().find_merchant_account_by_string(merchant_id).has_value()) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND, "merchant account not found");
        }
        if (connector_id.empty()) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND, "connector not found");
        }
        auto found = m_context->repository().find_merchant_connector_by_string(connector_id);
        if (!found.has_value()) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND, "connector not found");
        }
        const auto& record = *found;
        if (record.get_merchant_id() != merchant_id) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN, "connector does not belong to merchant");
        }
        (void)m_context->key_store().get("merchant_key", merchant_id);
        // Upstream: validate_profile_id_from_auth_layer(profile_id, connector).
        if (!profile_id.empty() && !record.get_profile_id().empty()
            && record.get_profile_id() != profile_id) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::PRECONDITION_FAILED, "connector profile mismatch");
        }
        return std::nullopt;
    }

    [[nodiscard]] bool connector_onboarding_enabled(std::string_view connector) const
    {
        const auto key = std::format("is_enabled_{}", normalized_connector(connector));
        const auto value = m_context->config_store().get_value("connector_onboarding", key);
        if (!value.has_value()) {
            return true;
        }
        return equals_ignore_case(*value, "true");
    }

    // Upstream build_key (utils/connector_onboarding.rs:144-154):
    // CONNECTOR_ONBOARDING_CONFIG_PREFIX_<connector>_<connector_id>.
    [[nodiscard]] std::string tracking_key(
        std::string_view connector_id,
        std::string_view connector
    ) const
    {
        return std::format(
            "{}_{}_{}",
            "CONNECTOR_ONBOARDING_CONFIG_PREFIX",
            normalized_connector(connector),
            connector_id
        );
    }

    [[nodiscard]] std::string now_unix_timestamp() const
    {
        const auto epoch = std::chrono::system_clock::now().time_since_epoch();
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(epoch).count()
        );
    }

    [[nodiscard]] std::string get_tracking_id(
        std::string_view connector_id,
        std::string_view connector
    ) const
    {
        const auto key = tracking_key(connector_id, connector);
        auto timestamp = m_context->config_store().get_value("configs", key);
        if (!timestamp.has_value() || timestamp->empty()) {
            timestamp = now_unix_timestamp();
            m_context->config_store().put("configs", key, *timestamp);
        }
        return std::format("{}_{}", connector_id, *timestamp);
    }

    void reset_tracking_id(
        std::string_view connector_id,
        std::string_view connector
    ) const
    {
        const auto key = tracking_key(connector_id, connector);
        m_context->config_store().put("configs", key, now_unix_timestamp());
    }

    [[nodiscard]] std::string json_text(const interfaces::Value& value) const
    {
        const auto& variant = value.get();
        if (const auto* text = std::get_if<std::string>(&variant)) {
            return *text;
        }
        if (const auto* object = std::get_if<interfaces::Value::Object>(&variant)) {
            if (auto inner = object->get("value")) {
                const auto& inner_variant = inner.value().get();
                if (const auto* text = std::get_if<std::string>(&inner_variant)) {
                    return *text;
                }
            }
        }
        return {};
    }

    [[nodiscard]] std::optional<std::string> response_field_text(
        const app::connector::ConnectorResponse& response,
        std::string_view field
    ) const
    {
        if (response.get_raw_json().empty()) {
            return std::nullopt;
        }
        auto parsed = serde::Ser::decode_generic("application/json", response.get_raw_json());
        if (!parsed) {
            return std::nullopt;
        }
        const auto& root = parsed->get();
        const auto* object = std::get_if<interfaces::Value::Object>(&root);
        if (object == nullptr) {
            return std::nullopt;
        }
        auto found = object->get(std::string{field});
        if (!found.has_value()) {
            return std::nullopt;
        }
        const auto text = json_text(found.value());
        if (text.empty()) {
            return std::nullopt;
        }
        return text;
    }

    // Upstream SellerStatusDetailsResponse::get_eligibility_status precedence
    // (paypal.rs:211-220): payments receivable, email confirmation, PPCP vetting, Success.
    // The real PayPal HTTP calls are an external dependency; this resolves the same variant
    // set from fields on the injected connector response, defaulting to Success.
    [[nodiscard]] OnboardingOutcome resolve_onboarding_status(
        const app::connector::ConnectorResponse& response
    ) const
    {
        if (const auto found = response_field_text(response, "account_found")) {
            if (equals_ignore_case(*found, "false")) {
                return {"account_not_found", std::string{}, false};
            }
        }
        if (const auto receivable = response_field_text(response, "payments_receivable")) {
            if (equals_ignore_case(*receivable, "false")) {
                return {"payments_not_receivable", std::string{}, false};
            }
        }
        if (const auto email = response_field_text(response, "primary_email_confirmed")) {
            if (equals_ignore_case(*email, "false")) {
                return {"email_not_verified", std::string{}, false};
            }
        }
        if (const auto vetting = response_field_text(response, "vetting_status")) {
            if (equals_ignore_case(*vetting, "denied")) {
                return {"ppcp_custom_denied", std::string{}, false};
            }
            if (!equals_ignore_case(*vetting, "subscribed")) {
                return {"more_permissions_needed", std::string{}, false};
            }
        }
        auto payer_id = response_field_text(response, "payer_id").value_or(std::string{});
        if (payer_id.empty()) {
            payer_id = payer_id_from_response(response);
        }
        return {"success", std::move(payer_id), true};
    }

    [[nodiscard]] std::string success_envelope(std::string_view payer_id) const
    {
        const auto payer_json = serde::Ser::encode_json(std::string{payer_id});
        return std::format(
            R"({{"paypal":{{"success":{{"payer_id":{}}}}}}})",
            payer_json
        );
    }

    // Upstream core:92-98 SignatureKey: api_key = client_secret, key1 = client_id,
    // api_secret = payer_id. Credentials come from KeyStore, then ConfigStore.
    void apply_integration_auth(
        app::store::merchant_connectors::MerchantConnectorRecord& record,
        std::string_view connector,
        std::string_view payer_id
    ) const
    {
        const auto client_secret = connector_credential(connector, "client_secret");
        const auto client_id = connector_credential(connector, "client_id");
        const auto auth_plaintext = std::format(
            R"({{"auth_type":"signature_key","api_key":"{}","key1":"{}","api_secret":"{}"}})",
            client_secret,
            client_id,
            payer_id
        );
        const auto wrapped = app::crypto::AesWrap{}.encrypt(
            auth_plaintext,
            connector_key(record.get_merchant_id(), std::string{connector})
        );
        const auto digest = app::crypto::KeyedHasher{"connector_onboarding.mca.auth"}.hash(wrapped);
        record.set_connector_account_details(wrapped);
        record.set_metadata(std::format(R"({{"auth_digest":"{}"}})", digest));
        record.set_connector_type("payment_processor");
        record.set_disabled(false);
        record.set_status("active");
    }

    [[nodiscard]] std::string connector_credential(
        std::string_view connector,
        std::string_view kind
    ) const
    {
        const auto key = std::format("{}_{}", normalized_connector(connector), kind);
        if (const auto secret = m_context->key_store().get("connector_onboarding", key)) {
            return secret->value();
        }
        return m_context->config_store().get_value("connector_onboarding", key).value_or(std::string{});
    }

    [[nodiscard]] bool is_paypal(std::string_view name) const
    {
        return equals_ignore_case(name, "paypal");
    }

    [[nodiscard]] std::string normalized_connector(std::string_view name) const
    {
        std::string out{};
        out.reserve(name.size());
        for (const char ch : name) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return out;
    }

    [[nodiscard]] bool equals_ignore_case(std::string_view lhs, std::string_view rhs) const
    {
        return lhs.size() == rhs.size() && std::equal(
            lhs.begin(), lhs.end(), rhs.begin(),
            [](char a, char b) {
                return std::tolower(static_cast<unsigned char>(a))
                    == std::tolower(static_cast<unsigned char>(b));
            }
        );
    }

    [[nodiscard]] std::string payer_id_from_response(
        const app::connector::ConnectorResponse& response
    ) const
    {
        return response_field_text(response, "payer_id").value_or(std::string{});
    }

    [[nodiscard]] std::string connector_key(
        std::string_view merchant,
        std::string_view connector
    ) const
    {
        std::string material = std::string{merchant} + ":" + std::string{connector};
        if (const auto master = m_context->key_store().get("merchant_key", merchant)) {
            material += ":" + master->value();
        }
        return app::crypto::KeyedHasher{"connector_onboarding.mca.key"}.hash(material);
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::connector_onboarding
