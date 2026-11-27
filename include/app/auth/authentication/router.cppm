export module app_auth:authentication_handler;

import std;
import interfaces;
import core_router;
import core_behavior;
import app_context;
import :authentication_input;
import :authentication_result;
import :authentication_runtime;
import :authentication_handlers_sync_redirect;
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
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

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

class EligibilityAuthenticateHandlers
{
public:
    EligibilityAuthenticateHandlers(
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
                Route{"eligibility"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_authentication_eligibility(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch authentication_eligibility")
            )
        );
        Router{router, "/authentication"}.add_router(
            Router{router, ":authentication_id"}.add_route(
                Route{"authenticate"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_authentication_authenticate(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch authentication_authenticate")
            )
        );
    }

    // routes/authentication.rs:51-90 -> unified_authentication_service.rs:1032-1351
    void handle_authentication_eligibility(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        using Dto = ::app::dto::authentication::AuthenticationEligibilityRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        m_policy.set_body_secret(input->get_client_secret());
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "authentication_eligibility", kAuthenticationSdkAuthSpec)) {
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
        m_runtime.eligibility(*input, ctx).handle(sink);
    }

    // routes/authentication.rs:94-135 -> unified_authentication_service.rs:1354-1518
    void handle_authentication_authenticate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        using Dto = ::app::dto::authentication::AuthenticationAuthenticateRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        m_policy.set_body_secret(input->get_client_secret());
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "authentication_authenticate", kAuthenticationSdkAuthSpec)) {
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
        m_runtime.authenticate(*input, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntime& m_runtime;
    AuthenticationPolicy& m_policy;
    core::behavior::RequestSupport<AuthenticationPolicy>& m_request_support;
};

class EligibilityCheckRetrieveHandlers
{
public:
    EligibilityCheckRetrieveHandlers(
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
                Route{"eligibility-check"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_authentication_eligibility_check(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch authentication_eligibility_check")
            )
        );
        Router{router, "/authentication"}.add_router(
            Router{router, ":authentication_id"}.add_route(
                Route{"eligibility-check"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_authentication_retrieve_eligibility_check(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch authentication_retrieve_eligibility_check")
            )
        );
    }

    // routes/authentication.rs:139-180 -> unified_authentication_service.rs:1685-1734
    void handle_authentication_eligibility_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        using Dto = ::app::dto::authentication::AuthenticationEligibilityCheckRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        m_policy.set_body_secret(input->get_client_secret());
        if (const auto auth_error = m_request_support.authorize(
                req,
                "authentication_eligibility_check",
                kAuthenticationSdkAuthSpec
            )) {
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
        m_runtime.eligibility_check(*input, ctx).handle(sink);
    }

    // routes/authentication.rs:184-212 -> unified_authentication_service.rs:1737-1790
    void handle_authentication_retrieve_eligibility_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(
                req,
                "authentication_retrieve_eligibility_check",
                kAuthenticationRetrieveAuthSpec
            )) {
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
        m_runtime.retrieve_eligibility_check(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntime& m_runtime;
    AuthenticationPolicy& m_policy;
    core::behavior::RequestSupport<AuthenticationPolicy>& m_request_support;
};

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

class AuthenticationRouteHandlers
{
public:
    explicit AuthenticationRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_policy{context},
        m_request_support{m_policy},
        m_create{context, m_runtime, m_policy, m_request_support},
        m_eligibility_authenticate{context, m_runtime, m_policy, m_request_support},
        m_eligibility_check_retrieve{context, m_runtime, m_policy, m_request_support},
        m_sync_redirect{context, m_runtime, m_policy, m_request_support},
        m_session_token{context, m_runtime, m_policy, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_create.install(router);
        m_eligibility_authenticate.install(router);
        m_eligibility_check_retrieve.install(router);
        m_sync_redirect.install(router);
        m_session_token.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntime m_runtime; // SINGLE shared runtime
    AuthenticationPolicy m_policy;
    core::behavior::RequestSupport<AuthenticationPolicy> m_request_support;
    CreateHandlers m_create;
    EligibilityAuthenticateHandlers m_eligibility_authenticate;
    EligibilityCheckRetrieveHandlers m_eligibility_check_retrieve;
    SyncRedirectHandlers m_sync_redirect;
    SessionTokenHandlers m_session_token;
};

} // namespace app::handlers::authentication
