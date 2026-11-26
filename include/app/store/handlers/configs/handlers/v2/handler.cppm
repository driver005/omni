export module app_store:configs_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;

import :configs_input;
import :configs_result;
import :configs_runtime;

export namespace app::handlers::configs {

/**
 * v2 surface: upstream `web::scope("/v2/configs")` with the same four
 * operations (routes/app.rs:2427-2439).
 *
 * Each handle_* is a distinct copy of the thin adapter — v2 does not share a
 * method with v1, matching the spec's "one adapter per version" rule. Runtime
 * is owned once by the facade and shared by reference, so both versions act
 * on the same config store.
 */
class ConfigsHandlersV2
{
public:
    ConfigsHandlersV2(
        ::app::context::AppContext& context,
        ConfigsRuntime& runtime,
        ConfigsInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    /** Upstream `config_key_create` -> `set_config` (routes/configs.rs:16-35). */
    void handle_config_key_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input.
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:31) -> registry PLATFORM_ADMIN.
        if (const auto auth_error = m_input_support.authorize_operation(req, "config_key_create")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // web::Json<Config> (routes/configs.rs:20): both fields are required strings.
        const auto body = req.body_as_string();
        auto parsed = serde::Ser::deserialize<ConfigRequestWire>("application/json", body);
        if (!parsed) {
            const auto error = deserialization_error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ConfigKeyCreateInput input{*parsed};

        // 2. Input -> Result.
        auto result = m_runtime.create(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    /** Upstream `config_key_retrieve` -> `read_config` (routes/configs.rs:36-55). */
    void handle_config_key_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input.
        // server_wrap auth: &InternalApiKeyAuth(ADMIN_API_AUTH) (routes/configs.rs:51).
        if (const auto auth_error =
                m_input_support.authorize_operation(req, "config_key_retrieve")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // web::Path<String> key (routes/configs.rs:40); never read from the body.
        const auto config_key = m_input_support.config_key_from_request(req);
        ConfigKeyRetrieveInput input{config_key};

        // 2. Input -> Result.
        auto result = m_runtime.retrieve(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    /** Upstream `config_key_update` -> `update_config` (routes/configs.rs:56-78). */
    void handle_config_key_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input.
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:74) -> registry PLATFORM_ADMIN.
        if (const auto auth_error = m_input_support.authorize_operation(req, "config_key_update")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // web::Json<ConfigUpdate> with key(skip_deserializing): decode only `value`.
        const auto body = req.body_as_string();
        auto parsed = serde::Ser::deserialize<ConfigUpdateRequestWire>("application/json", body);
        if (!parsed) {
            const auto error = deserialization_error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // payload.key = path.into_inner() (routes/configs.rs:65-66) — the path wins.
        const auto config_key = m_input_support.config_key_from_request(req);
        ConfigKeyUpdateInput input{*parsed, config_key};

        // 2. Input -> Result.
        auto result = m_runtime.update(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    /** Upstream `config_key_delete` -> `config_delete` (routes/configs.rs:80-99). */
    void handle_config_key_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input.
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:95) -> registry PLATFORM_ADMIN.
        if (const auto auth_error = m_input_support.authorize_operation(req, "config_key_delete")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // web::Path<String> key (routes/configs.rs:84).
        const auto config_key = m_input_support.config_key_from_request(req);
        ConfigKeyDeleteInput input{config_key};

        // 2. Input -> Result.
        auto result = m_runtime.remove(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        Router{router, "v2/configs"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_config_key_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch config_key_create (v2)")
            )
            .add_route(
                Route{":key"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_config_key_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch config_key_retrieve (v2)")
            )
            .add_route(
                Route{":key"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_config_key_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch config_key_update (v2)")
            )
            .add_route(
                Route{":key"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_config_key_delete(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch config_key_delete (v2)")
            );
    }

private:
    /** Upstream IR_06 / invalid_request: message in `message`, code in `error`. */
    [[nodiscard]] static core::error::AppError deserialization_error()
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_JSON,
            "failed to deserialize json body"
        );
        error.set_kind_name("IR_06");
        return error;
    }

    ::app::context::AppContext& m_context;
    ConfigsRuntime& m_runtime;
    ConfigsInputSupport& m_input_support;
};

} // namespace app::handlers::configs
