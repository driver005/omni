module;
export module app_handlers:configs;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_metrics;
import app_store;

export namespace app::handlers::configs {

/**
 * Local substitutes for the upstream Cargo features that select the config
 * scope and the admin auth type. This port has no Cargo `#[cfg]`, so these
 * constexpr gates are the exact local equivalent: upstream registers exactly
 * one scope per build (routes/app.rs:2412-2424 v1 / :2427-2439 v2) and selects
 * `AdminApiAuth` vs `V2AdminApiAuth` at compile time (routes/configs.rs:11-14).
 * Both feature sets are compiled in here, matching an upstream release build
 * with v1 and v2 enabled. Both admin auth variants resolve to the same
 * admin-level rule in the shared registry (app/auth/registry.cppm:59-62).
 */
inline constexpr bool kV1 = true; // /configs
inline constexpr bool kV2 = true; // /v2/configs

/**
 * @brief Local mirror of upstream `api_types::Config{key: String, value: String}`
 *        (`docs/hyperswitch/crates/router/src/types/api/configs.rs:1-5`).
 *        Both fields are required strings: reflect-cpp treats a missing or
 *        non-string field as a deserialization failure, exactly like serde.
 */
class ConfigRequestWire
{
public:
    void set_key(std::string value) { key = std::move(value); }
    [[nodiscard]] const std::string& get_key() const noexcept { return key; }
    void set_value(std::string value) { value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }

    std::string key{};
    std::string value{};
};

/**
 * @brief Local mirror of upstream `api_types::ConfigUpdate`
 *        (`docs/hyperswitch/crates/router/src/types/api/configs.rs:7-12`).
 *        Upstream's `key` is `#[serde(skip_deserializing)]` and is overwritten
 *        from the web::Path<String> in the route (routes/configs.rs:61-66), so
 *        this wire shape intentionally carries only `value`: any `key` in the
 *        body is ignored, exactly as upstream does.
 */
class ConfigUpdateRequestWire
{
public:
    void set_value(std::string value) { value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }

    std::string value{};
};

} // namespace app::handlers::configs

// Serialization metadata documents the exact upstream wire fields. Decoding is
// done by serde::Ser::deserialize<T>, which uses reflect-cpp on the public fields.
template<>
struct serde::Serializable<::app::handlers::configs::ConfigRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"key", &::app::handlers::configs::ConfigRequestWire::get_key, &::app::handlers::configs::ConfigRequestWire::set_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"value", &::app::handlers::configs::ConfigRequestWire::get_value, &::app::handlers::configs::ConfigRequestWire::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "config_request_wire"; }
};

template<>
struct serde::Serializable<::app::handlers::configs::ConfigUpdateRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"value", &::app::handlers::configs::ConfigUpdateRequestWire::get_value, &::app::handlers::configs::ConfigUpdateRequestWire::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "config_update_request_wire"; }
};

export namespace app::handlers::configs {

class ConfigsRouteHandlers
{
public:
    explicit ConfigsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    /** Upstream `config_key_create` -> `set_config` (routes/configs.rs:16-35; core/configs.rs:22-34). */
    void handle_config_key_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:31) -> registry PLATFORM_ADMIN
        // (app/auth/registry.cppm:59).
        if (const auto auth_error = authorize_operation(req, "config_key_create")) {
            send_error(res, send, *auth_error);
            return;
        }

        // web::Json<Config> (routes/configs.rs:20): both fields are required strings
        // (types/api/configs.rs:1-5). A malformed body or missing/wrong-typed field is
        // rejected by the upstream JsonConfig error handler (router/src/utils.rs:78-121).
        const auto body = req.body_as_string();
        auto parsed = serde::Ser::deserialize<ConfigRequestWire>("application/json", body);
        if (!parsed) {
            send_json_deserialization_error(res, send);
            return;
        }

        // set_config: store.insert_config(ConfigNew { key: config.key, config: config.value })
        // .to_duplicate_response(DuplicateConfig) (core/configs.rs:24-31). The DB-backed
        // repository enforces the unique natural key (app/store/repository.cppm:1239-1243).
        app::store::configs::ConfigRecord record{};
        record.set_config_key(parsed->get_key());
        record.set_config_value(parsed->get_value());
        auto created = m_context->repository().create_config(record);
        if (!created) {
            send_duplicate_config(res, send);
            return;
        }

        // #[instrument(flow = ?Flow::CreateConfigKey)] (routes/configs.rs:16) — local
        // metrics substitute. LockAction::NotApplicable (routes/configs.rs:32) means no lock.
        record_flow("CreateConfigKey");

        // ApplicationResponse::Json(config.foreign_into()) (core/configs.rs:33) emits the
        // stored row as {key, value}; both are strings (types/transformers.rs:560-567).
        send_json(res, send, make_response(created->get_config_key(), created->get_config_value()));
    }

    /** Upstream `config_key_retrieve` -> `read_config` (routes/configs.rs:36-55; core/configs.rs:36-44). */
    void handle_config_key_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // server_wrap auth: &InternalApiKeyAuth(ADMIN_API_AUTH) (routes/configs.rs:51) ->
        // registry API_KEY (app/auth/registry.cppm:61).
        if (const auto auth_error = authorize_operation(req, "config_key_retrieve")) {
            send_error(res, send, *auth_error);
            return;
        }

        // web::Path<String> key (routes/configs.rs:40); never read from the body.
        const auto config_key = config_key_from_request(req);

        // read_config: store.find_config_by_key_optional(key)
        // .to_not_found_response(ConfigNotFound)?.ok_or(ConfigNotFound)?
        // (core/configs.rs:38-42).
        const auto record = m_context->repository().find_config_by_key(config_key);
        if (!record) {
            send_config_not_found(res, send);
            return;
        }

        record_flow("ConfigKeyFetch");

        // Json(config.foreign_into()) (core/configs.rs:43) uses the stored key/value.
        send_json(res, send, make_response(record->get_config_key(), record->get_config_value()));
    }

    /** Upstream `config_key_update` -> `update_config` (routes/configs.rs:56-78; core/configs.rs:46-56). */
    void handle_config_key_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:74) -> registry PLATFORM_ADMIN
        // (app/auth/registry.cppm:62).
        if (const auto auth_error = authorize_operation(req, "config_key_update")) {
            send_error(res, send, *auth_error);
            return;
        }

        // web::Json<ConfigUpdate> with key(skip_deserializing) (routes/configs.rs:61;
        // types/api/configs.rs:7-12): decode only `value`, ignore any body `key`.
        const auto body = req.body_as_string();
        auto parsed = serde::Ser::deserialize<ConfigUpdateRequestWire>("application/json", body);
        if (!parsed) {
            send_json_deserialization_error(res, send);
            return;
        }

        // payload.key = path.into_inner() (routes/configs.rs:65-66) — the path wins.
        const auto config_key = config_key_from_request(req);

        // update_config: store.update_config_by_key(&config_update.key, config_update.foreign_into())
        // .to_not_found_response(ConfigNotFound)? (core/configs.rs:51-54). foreign_into maps
        // only `value` into the storage update (types/transformers.rs:569-575).
        auto existing = m_context->repository().find_config_by_key(config_key);
        if (!existing) {
            send_config_not_found(res, send);
            return;
        }
        existing->set_config_value(parsed->get_value());
        if (!m_context->repository().update_config(*existing)) {
            // A zero-row update is the local equivalent of the store's not-found error.
            send_config_not_found(res, send);
            return;
        }

        record_flow("ConfigKeyUpdate");

        // Json(config.foreign_into()) of the updated row (core/configs.rs:55).
        send_json(res, send, make_response(existing->get_config_key(), existing->get_config_value()));
    }

    /** Upstream `config_key_delete` -> `config_delete` (routes/configs.rs:80-99; core/configs.rs:58-65). */
    void handle_config_key_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:95) -> registry PLATFORM_ADMIN
        // (app/auth/registry.cppm:60).
        if (const auto auth_error = authorize_operation(req, "config_key_delete")) {
            send_error(res, send, *auth_error);
            return;
        }

        // web::Path<String> key (routes/configs.rs:84).
        const auto config_key = config_key_from_request(req);

        // config_delete: store.delete_config_by_key(&key)
        // .to_not_found_response(ConfigNotFound)? (core/configs.rs:60-63).
        const auto record = m_context->repository().find_config_by_key(config_key);
        if (!record) {
            send_config_not_found(res, send);
            return;
        }
        if (!m_context->repository().remove_config(record->get_id())) {
            send_config_not_found(res, send);
            return;
        }

        record_flow("ConfigKeyDelete");

        // Json(config.foreign_into()) of the deleted row (core/configs.rs:64).
        send_json(res, send, make_response(record->get_config_key(), record->get_config_value()));
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Upstream v1: `web::scope("/configs")` with resource("/") POST and
        // resource("/{key}") GET/POST/DELETE (routes/app.rs:2412-2424).
        if constexpr (kV1) {
            Router{router, "/configs"}
                .add_route(
                    Route{"/"}
                        .post([this](
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                                  handle_config_key_create(req, res, std::move(send));
                              })
                        .summary("Hyperswitch config_key_create")
                )
                .add_route(
                    Route{"/:key"}
                        .get([this](
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                                 handle_config_key_retrieve(req, res, std::move(send));
                             })
                        .summary("Hyperswitch config_key_retrieve")
                )
                .add_route(
                    Route{"/:key"}
                        .post([this](
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                                  handle_config_key_update(req, res, std::move(send));
                              })
                        .summary("Hyperswitch config_key_update")
                )
                .add_route(
                    Route{"/:key"}
                        .delt([this](
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                                  handle_config_key_delete(req, res, std::move(send));
                              })
                        .summary("Hyperswitch config_key_delete")
                );
        }

        // Upstream v2: `web::scope("/v2/configs")` with the same four ops
        // (routes/app.rs:2427-2439).
        if constexpr (kV2) {
            Router{router, "/v2/configs"}
                .add_route(
                    Route{"/"}
                        .post([this](
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                                  handle_config_key_create(req, res, std::move(send));
                              })
                        .summary("Hyperswitch config_key_create (v2)")
                )
                .add_route(
                    Route{"/:key"}
                        .get([this](
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                                 handle_config_key_retrieve(req, res, std::move(send));
                             })
                        .summary("Hyperswitch config_key_retrieve (v2)")
                )
                .add_route(
                    Route{"/:key"}
                        .post([this](
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                                  handle_config_key_update(req, res, std::move(send));
                              })
                        .summary("Hyperswitch config_key_update (v2)")
                )
                .add_route(
                    Route{"/:key"}
                        .delt([this](
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send
                              ) {
                                  handle_config_key_delete(req, res, std::move(send));
                              })
                        .summary("Hyperswitch config_key_delete (v2)")
                );
        }
    }

private:
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    [[nodiscard]] std::string config_key_from_request(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        // v1 scope is /configs/:key; v2 scope is /v2/configs/:key.
        return std::string{segments.at(is_v2(req) ? 2U : 1U).value_or("")};
    }

    /** Local substitute for the upstream `#[instrument(flow = ?Flow::...)]` tracing span. */
    void record_flow(std::string_view flow) const
    {
        m_context->metrics().record_flow(flow);
        m_context->metrics().counter(flow).increment();
    }

    /**
     * Exact upstream ErrorResponse wire shape (api_models/src/errors/types.rs:41-70):
     * `{"type":"invalid_request","message":"...","code":"HE_01"}`.
     */
    void send_api_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        interfaces::io::types::Status status,
        std::string_view message,
        std::string_view code
    ) const
    {
        const auto body = std::format(
            R"({{"type":"invalid_request","message":"{}","code":"{}"}})",
            json_escape(message),
            code
        );
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    /** DuplicateConfig -> BadRequest HE_01 (hyperswitch_domain_models/errors/api_error_response.rs:444-446). */
    void send_duplicate_config(interfaces::io::IResponse& res, std::function<void()> send) const
    {
        send_api_error(
            res, send, interfaces::io::types::Status::BAD_REQUEST,
            "The config with the specified key already exists in our records", "HE_01"
        );
    }

    /** ConfigNotFound -> NotFound HE_02 (hyperswitch_domain_models/errors/api_error_response.rs:456-458). */
    void send_config_not_found(interfaces::io::IResponse& res, std::function<void()> send) const
    {
        send_api_error(
            res, send, interfaces::io::types::Status::NOT_FOUND,
            "Config key does not exist in our records.", "HE_02"
        );
    }

    /**
     * Upstream `custom_json_error_handler` (router/src/utils.rs:78-121): HTTP 400 with
     * `{"error":{"code":"IR_06","error_type":"invalid_request","message":"..."}}`.
     * Inner key order is alphabetical because upstream builds it with `serde_json::json!`
     * (BTreeMap; router/Cargo.toml:272 has no `preserve_order` feature). The message is a
     * local stand-in for serde_json's detailed decode error text.
     */
    void send_json_deserialization_error(interfaces::io::IResponse& res, std::function<void()> send) const
    {
        constexpr std::string_view body =
            R"({"error":{"code":"IR_06","error_type":"invalid_request","message":"failed to deserialize json body"}})";
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    /**
     * Build the canonical {key, value} response from stored fields only — never
     * echo the request. Both fields are JSON strings, matching `api::Config`
     * (types/api/configs.rs:1-5) and its transformer (types/transformers.rs:560-567).
     */
    [[nodiscard]] std::string make_response(std::string_view key, std::string_view value) const
    {
        serde::Value::Object fields{};
        fields["key"] = serde::Value{std::string{key}};
        fields["value"] = serde::Value{std::string{value}};
        return serde::Ser::encode_json(serde::Value{std::move(fields)});
    }

    [[nodiscard]] std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch : value) {
            switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
            }
        }
        return out;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        // Auth failures are produced by the shared authorizer (app/auth/authorizer.cppm);
        // the envelope is that shared stack's contract, not this module's.
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::configs
