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
        if (const auto auth_error =
                m_request_support
                    .authorize(req, "authentication_create", kAuthenticationCreateAuthSpec)) {
            auth_error->send(res, send);
            return;
        }
        const auto authorized = m_policy.authorized();
        const auto merchant_id = m_request_support.scope(req).merchant_id;
        if (const auto scope_error = m_request_support.scope_error({merchant_id})) {
            scope_error->send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        using Input = ::app::dto::authentication::AuthenticationCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        AuthenticationCreateInput input{*parsed, std::string{merchant_id}, authorized};
        auto result = m_runtime.create(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        auto encoded = result->serialize(create_wire_fields());
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
