export module app_auth:authentication_handlers_eligibility_authenticate;

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
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        using Input = ::app::dto::authentication::AuthenticationEligibilityRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        m_policy.set_body_secret(parsed->get_client_secret());
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "authentication_eligibility", kAuthenticationSdkAuthSpec)) {
            auth_error->send(res, send);
            return;
        }
        const auto authorized = m_policy.authorized();
        const auto id = m_request_support.path(req, 1);
        if (id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "authentication id missing in path"
            );
            error.send(res, send);
            return;
        }
        AuthenticationEligibilityInput input{*parsed, id, authorized, std::string{body}};
        auto result = m_runtime.eligibility(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // routes/authentication.rs:94-135 -> unified_authentication_service.rs:1354-1518
    void handle_authentication_authenticate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        using Input = ::app::dto::authentication::AuthenticationAuthenticateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        m_policy.set_body_secret(parsed->get_client_secret());
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "authentication_authenticate", kAuthenticationSdkAuthSpec)) {
            auth_error->send(res, send);
            return;
        }
        const auto authorized = m_policy.authorized();
        const auto id = m_request_support.path(req, 1);
        if (id.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "authentication id missing in path"
            );
            error.send(res, send);
            return;
        }
        auto authentication_id_value = ::app::dto::authentication::AuthenticationIdValue{};
        authentication_id_value.set_value(id);
        parsed->set_authentication_id(authentication_id_value);
        AuthenticationAuthenticateInput input{*parsed, id, authorized};
        auto result = m_runtime.authenticate(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        auto encoded = result->serialize(authenticate_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntime& m_runtime;
    AuthenticationPolicy& m_policy;
    core::behavior::RequestSupport<AuthenticationPolicy>& m_request_support;
};

} // namespace app::handlers::authentication
