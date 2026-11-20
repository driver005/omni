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

class ConfigsRouteHandlers
{
public:
    explicit ConfigsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_config_key_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_operation(req, "config_key_create");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream `Config{key, value}` is deserialized by serde; both fields are required.
        // No app_dto mirror exists for its arbitrary-JSON `value`, so normalize into a
        // local serde::Value shape instead of echoing the request body.
        const auto body = req.body_as_string();
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "invalid config body"));
            return;
        }
        const auto config_key = field_string(*parsed, "key");
        if (!config_key || config_key->empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::VALIDATION_FAILED, "config key is required"));
            return;
        }
        const auto value = field_value(*parsed, "value");
        if (!value) {
            send_error(res, send, make_error(core::error::AppErrorKind::VALIDATION_FAILED, "config value is required"));
            return;
        }

        // Persist through the repository's DB-backed config store. create_config enforces
        // the unique natural key and returns DuplicateConfig-mapped ALREADY_EXISTS on conflict.
        app::store::configs::ConfigRecord record{};
        record.set_config_key(*config_key);
        record.set_config_value(serde::Ser::encode_json(*value));
        auto created = m_context->repository().create_config(record);
        if (!created) {
            send_error(res, send, created.error());
            return;
        }

        const auto text = make_response(created->get_config_key(), *value);

        // Upstream flow instrumentation via server_wrap; LockAction::NotApplicable
        // means no lock is acquired for this operation.
        m_context->metrics().record_flow("CreateConfigKey");
        m_context->metrics().counter("CONFIG_KEY_CREATED").increment();

        send_json(res, send, text, true);
    }

    void handle_config_key_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Distinct rule: upstream wraps ADMIN_API_AUTH in InternalApiKeyAuth for GET only.
        const auto auth_error = authorize_operation(req, "config_key_retrieve");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream takes the key from web::Path<String>, never from the body.
        const auto config_key = config_key_from_request(req);
        const auto record = m_context->repository().find_config_by_key(config_key);
        if (!record) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "config not found"));
            return;
        }
        const auto value = serde::Ser::decode_generic("application/json", record->get_config_value());
        if (!value) {
            send_error(res, send, make_error(core::error::AppErrorKind::INTERNAL_ERROR, "stored config is malformed"));
            return;
        }
        const auto text = make_response(config_key, *value);

        m_context->metrics().record_flow("ConfigKeyFetch");
        m_context->metrics().counter("CONFIG_KEY_FETCHED").increment();

        send_json(res, send, text, true);
    }

    void handle_config_key_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_operation(req, "config_key_update");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream ConfigUpdate has key(skip_deserializing); the :key path segment wins.
        const auto body = req.body_as_string();
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "invalid config body"));
            return;
        }
        const auto config_key = config_key_from_request(req);
        if (config_key.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "config key is required"));
            return;
        }
        const auto value = field_value(*parsed, "value");
        if (!value) {
            send_error(res, send, make_error(core::error::AppErrorKind::VALIDATION_FAILED, "config value is required"));
            return;
        }
        auto existing = m_context->repository().find_config_by_key(config_key);
        if (!existing) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "config not found"));
            return;
        }
        existing->set_config_value(serde::Ser::encode_json(*value));
        if (!m_context->repository().update_config(*existing)) {
            send_error(res, send, make_error(core::error::AppErrorKind::INTERNAL_ERROR, "failed to update config"));
            return;
        }

        const auto text = make_response(config_key, *value);

        m_context->metrics().record_flow("ConfigKeyUpdate");
        m_context->metrics().counter("CONFIG_KEY_UPDATED").increment();

        send_json(res, send, text, true);
    }

    void handle_config_key_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_operation(req, "config_key_delete");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto config_key = config_key_from_request(req);
        const auto record = m_context->repository().find_config_by_key(config_key);
        if (!record) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "config not found"));
            return;
        }
        const auto value = serde::Ser::decode_generic("application/json", record->get_config_value());
        if (!value) {
            send_error(res, send, make_error(core::error::AppErrorKind::INTERNAL_ERROR, "stored config is malformed"));
            return;
        }
        if (!m_context->repository().remove_config(record->get_id())) {
            send_error(res, send, make_error(core::error::AppErrorKind::INTERNAL_ERROR, "failed to delete config"));
            return;
        }

        const auto text = make_response(config_key, *value);

        m_context->metrics().record_flow("ConfigKeyDelete");
        m_context->metrics().counter("CONFIG_KEY_DELETED").increment();

        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
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

        // Upstream exposes the same four operations under /v2/configs.
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

private:
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] core::error::AppError make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
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

    // Returns the raw JSON value of a Config field (upstream `value` is serde_json::Value).
    [[nodiscard]] std::optional<serde::Value> field_value(
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
        return *field;
    }

    // Build a typed {key, value} response from canonical fields only; never echo the
    // request body. The value is the original JSON value, not a stringified copy.
    [[nodiscard]] std::string make_response(std::string_view key, const serde::Value& value) const
    {
        serde::Value::Object fields{};
        fields["key"] = serde::Value{std::string{key}};
        fields["value"] = value;
        return serde::Ser::encode_json(serde::Value{std::move(fields)});
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

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::configs
