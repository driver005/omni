export module app_handlers_authentication:handlers_session_token;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;
import :runtime;

export namespace app::handlers::authentication {

class SessionTokenHandlers
{
public:
    SessionTokenHandlers(app::context::AppContext& ctx, AuthenticationRuntime& runtime) noexcept :
        m_context{ctx},
        m_input_support{ctx},
        m_runtime{runtime}
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
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::authentication::AuthenticationSessionTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto authorized = m_input_support.authorize_sdk(
            req,
            "authentication_session_token",
            body,
            parsed->get_client_secret()
        );
        if (authorized.error) {
            const auto& error = *authorized.error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto id = m_input_support.auth_id_from_path(req, 1);
        if (!id.has_value()) {
            const auto& error = id.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        auto authentication_id_value = app::dto::authentication::AuthenticationIdValue{};
        authentication_id_value.set_value(*id);
        parsed->set_authentication_id(authentication_id_value);
        AuthenticationSessionTokenInput input{*parsed, *id, authorized};
        auto result = m_runtime.session_token(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

private:
    app::context::AppContext& m_context;
    AuthenticationInputSupport m_input_support;
    AuthenticationRuntime& m_runtime;
};

} // namespace app::handlers::authentication
