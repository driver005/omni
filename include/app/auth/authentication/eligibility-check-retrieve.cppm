export module app_auth:authentication_handlers_eligibility_check_retrieve;

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

class EligibilityCheckRetrieveHandlers
{
public:
    EligibilityCheckRetrieveHandlers(
        ::app::context::AppContext& ctx,
        AuthenticationRuntime& runtime
    ) noexcept :
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
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        using Input = ::app::dto::authentication::AuthenticationEligibilityCheckRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        const auto authorized = m_input_support.authorize_sdk(
            req,
            "authentication_eligibility_check",
            body,
            parsed->get_client_secret()
        );
        if (authorized.error) {
            const auto& error = *authorized.error;
            error.send(res, send);
            return;
        }
        const auto id = m_input_support.auth_id_from_path(req, 1);
        if (!id.has_value()) {
            const auto& error = id.error();
            error.send(res, send);
            return;
        }
        auto authentication_id_value = ::app::dto::authentication::AuthenticationIdValue{};
        authentication_id_value.set_value(*id);
        parsed->set_authentication_id(authentication_id_value);
        AuthenticationEligibilityCheckInput input{*parsed, *id, authorized, std::string{body}};
        auto result = m_runtime.eligibility_check(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // routes/authentication.rs:184-212 -> unified_authentication_service.rs:1737-1790
    void handle_authentication_retrieve_eligibility_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = m_input_support.authorize_api_key(
            req,
            "authentication_retrieve_eligibility_check",
            ::app::auth::AppPermission::PAYMENT_READ
        );
        if (authorized.error) {
            const auto& error = *authorized.error;
            error.send(res, send);
            return;
        }
        const auto id = m_input_support.auth_id_from_path(req, 1);
        if (!id.has_value()) {
            const auto& error = id.error();
            error.send(res, send);
            return;
        }
        AuthenticationRetrieveEligibilityCheckInput input{*id, authorized};
        auto result = m_runtime.retrieve_eligibility_check(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationInputSupport m_input_support;
    AuthenticationRuntime& m_runtime;
};

} // namespace app::handlers::authentication
