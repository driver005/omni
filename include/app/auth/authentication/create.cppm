export module app_auth:authentication_handlers_create;

import std;
import interfaces;
import core_router;
import core_behavior;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :authentication_input;
import :authentication_result;
import :authentication_runtime;

export namespace app::handlers::authentication {

class CreateHandlers
{
public:
    CreateHandlers(
        ::app::context::AppContext& ctx,
        AuthenticationRuntime& runtime,
        AuthenticationPolicy& policy,
        core::behavior::RequestSupport<AuthenticationPolicy>& request_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_policy{policy},
        m_request_support{request_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/authentication"}.add_route(
            Route{""}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_authentication_create(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch authentication_create")
        );
    }

    // routes/authentication.rs:20-47 -> unified_authentication_service.rs:712-875
    void handle_authentication_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "authentication_create", kAuthenticationCreateAuthSpec)) {
            auth_error->send(sink);
            return;
        }
        const auto ctx = m_request_support.context(req);
        if (const auto scope_error = m_request_support.scope_error({ctx.merchant_id})) {
            scope_error->send(sink);
            return;
        }
        using Dto = ::app::dto::authentication::AuthenticationCreateRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        m_runtime.create(*input, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntime& m_runtime;
    AuthenticationPolicy& m_policy;
    core::behavior::RequestSupport<AuthenticationPolicy>& m_request_support;
};

} // namespace app::handlers::authentication
