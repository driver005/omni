export module app_auth:authentication_handlers_session_token;

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

class SessionTokenHandlers
{
public:
    SessionTokenHandlers(
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
        Router{router, "/authentication"}.add_router(
            Router{router, ":authentication_id"}.add_route(
                Route{"enabled_authn_methods_token"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_authentication_session_token(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch authentication_session_token")
            )
        );
    }

    // routes/authentication.rs:292-328 -> unified_authentication_service.rs:2516-2583
    void handle_authentication_session_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        using Dto = ::app::dto::authentication::AuthenticationSessionTokenRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        m_policy.set_body_secret(input->get_client_secret());
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "authentication_session_token", kAuthenticationSdkAuthSpec)) {
            auth_error->send(sink);
            return;
        }
        const auto ctx = m_request_support.context(req);
        if (ctx.path_segments.size() < 2 || ctx.path_segments[1].empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "authentication id missing in path"
            );
            error.send(sink);
            return;
        }
        auto authentication_id_value = ::app::dto::authentication::AuthenticationIdValue{};
        authentication_id_value.set_value(ctx.path_segments[1]);
        input->set_authentication_id(authentication_id_value);
        m_runtime.session_token(*input, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntime& m_runtime;
    AuthenticationPolicy& m_policy;
    core::behavior::RequestSupport<AuthenticationPolicy>& m_request_support;
};

} // namespace app::handlers::authentication
