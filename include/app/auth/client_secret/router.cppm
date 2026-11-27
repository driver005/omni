export module app_auth:client_secret_handler;

import std;
import core_router;
import core_behavior;
import app_context;
import :client_secret_input;
import :client_secret_result;
import :client_secret_runtime;
import interfaces;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_keystore;
import app_locking;
import app_services;

export namespace app::handlers::client_secret {

class ClientSecretHandlersV2
{
public:
    ClientSecretHandlersV2(
        ::app::context::AppContext& context,
        ClientSecretRuntime& runtime,
        core::behavior::RequestSupport<ClientSecretPolicy>& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_request_support{request_support}
    {
    }

    void handle_client_secret_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_request_support.authorize(req, "client_secret_create", kClientSecretApiKeySpec)) {
            auth_error->send(sink);
            return;
        }

        using Dto = ::app::dto::client_secret::ClientSecretCreateRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        if (!input->get_resource_id().is_customer()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(sink);
            return;
        }
        m_runtime.create(*input, m_request_support.context(req)).handle(sink);
    }

    void handle_client_secret_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_request_support.authorize(req, "client_secret_delete", kClientSecretApiKeySpec)) {
            auth_error->send(sink);
            return;
        }

        // web::Path<String> id — routes/client_secret.rs:46,49.
        const auto ctx = m_request_support.context(req);
        if (ctx.path_segments.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing client secret id"
            );
            error.send(sink);
            return;
        }
        m_runtime.delete_client_secret(ctx).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/client-secret"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_client_secret_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch client_secret_create")
            )
            .add_route(
                Route{":id"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_client_secret_delete(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch client_secret_delete")
            );
    }

private:
    ::app::context::AppContext& m_context;
    ClientSecretRuntime& m_runtime;
    core::behavior::RequestSupport<ClientSecretPolicy>& m_request_support;
};

class ClientSecretRouteHandlers
{
public:
    explicit ClientSecretRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_policy{context},
        m_request_support{m_policy},
        m_v2{context, m_runtime, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    ClientSecretRuntime m_runtime;
    ClientSecretPolicy m_policy;
    core::behavior::RequestSupport<ClientSecretPolicy> m_request_support;
    ClientSecretHandlersV2 m_v2;
};

} // namespace app::handlers::client_secret
