export module app_store:configs_handlers_v1;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;

import :configs_input;
import :configs_result;
import :configs_runtime;

export namespace app::handlers::configs {

/**
 * v1 surface: upstream `web::scope("/configs")` with resource("/") POST and
 * resource("/{key}") GET/POST/DELETE (routes/app.rs:2412-2424).
 *
 * Each handle_* is a thin adapter: request -> DTO + CallContext -> runtime -> response.
 * Runtime is owned once by the facade and shared by reference. v1 and v2 each
 * carry their own copy of these adapters because upstream registers the same
 * four operations under both scopes.
 */
class ConfigsHandlersV1
{
public:
    ConfigsHandlersV1(
        ::app::context::AppContext& context,
        ConfigsRuntime& runtime,
        core::behavior::RequestSupport<ConfigsPolicy>& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_request_support{request_support}
    {
    }

    /** Upstream `config_key_create` -> `set_config` (routes/configs.rs:16-35). */
    void handle_config_key_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> DTO + CallContext.
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:31) -> registry PLATFORM_ADMIN.
        if (const auto auth_error = m_request_support.authorize(req, "config_key_create", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // web::Json<Config> (routes/configs.rs:20): pass parses + validates the DTO.
        auto dto = serde::Serializer<ConfigRequestWire>::pass(req, sink);
        if (!dto) {
            return;
        }
        m_runtime.create(*dto, m_request_support.context(req)).handle(sink);
    }

    /** Upstream `config_key_retrieve` -> `read_config` (routes/configs.rs:36-55). */
    void handle_config_key_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> DTO + CallContext.
        // server_wrap auth: &InternalApiKeyAuth(ADMIN_API_AUTH) (routes/configs.rs:51).
        if (const auto auth_error = m_request_support.authorize(req, "config_key_retrieve", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // web::Path<String> key (routes/configs.rs:40) is read from the context.
        m_runtime.retrieve(m_request_support.context(req)).handle(sink);
    }

    /** Upstream `config_key_update` -> `update_config` (routes/configs.rs:56-78). */
    void handle_config_key_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> DTO + CallContext.
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:74) -> registry PLATFORM_ADMIN.
        if (const auto auth_error = m_request_support.authorize(req, "config_key_update", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // web::Json<ConfigUpdate> with key(skip_deserializing): decode only `value`.
        auto dto = serde::Serializer<ConfigUpdateRequestWire>::pass(req, sink);
        if (!dto) {
            return;
        }

        // payload.key = path.into_inner() (routes/configs.rs:65-66) — the path wins.
        m_runtime.update(*dto, m_request_support.context(req)).handle(sink);
    }

    /** Upstream `config_key_delete` -> `config_delete` (routes/configs.rs:80-99). */
    void handle_config_key_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> DTO + CallContext.
        // server_wrap auth: &ADMIN_API_AUTH (routes/configs.rs:95) -> registry PLATFORM_ADMIN.
        if (const auto auth_error = m_request_support.authorize(req, "config_key_delete", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // web::Path<String> key (routes/configs.rs:84) is read from the context.
        m_runtime.remove(m_request_support.context(req)).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        Router{router, "configs"}
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
                    .summary("Hyperswitch config_key_create")
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
                    .summary("Hyperswitch config_key_retrieve")
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
                    .summary("Hyperswitch config_key_update")
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
                    .summary("Hyperswitch config_key_delete")
            );
    }

private:
    ::app::context::AppContext& m_context;
    ConfigsRuntime& m_runtime;
    core::behavior::RequestSupport<ConfigsPolicy>& m_request_support;
};

} // namespace app::handlers::configs
