export module app_auth:hypersense_handler;

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
import app_services;

import :hypersense_input;
import :hypersense_result;
import :hypersense_runtime;

export namespace app::handlers::hypersense {

class HypersenseRouteHandlers
{
public:
    // routes/hypersense.rs:28-31 — DashboardNoPermissionAuth, no body.
    void handle_get_hypersense_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        GetTokenInput input{
            m_input_support.bearer_token(req),
            m_request_support.scope(req).merchant_id
        };
        auto result = m_runtime.get_token(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        auto encoded = result->serialize(get_token_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    // routes/hypersense.rs:75 — NoAuth. The payload token is the only credential.
    void handle_verify_hypersense_token(
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
        using Input = ::app::dto::external_service_auth::ExternalVerifyTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        VerifyTokenInput input{*parsed};
        auto result = m_runtime.verify_token(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        auto encoded = result->serialize(HypersenseVerifyResponse::wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    // routes/hypersense.rs:51 — NoAuth. Successful core is bodyless StatusOk.
    void handle_signout_hypersense_token(
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
        using Input = ::app::dto::external_service_auth::ExternalSignoutTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        SignoutTokenInput input{*parsed};
        auto result = m_runtime.signout_token(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::vector<std::byte>{});
        send();
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "hypersense"}
            .add_route(
                Route{"token"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_hypersense_token(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_hypersense_token")
            )
            .add_route(
                Route{"verify_token"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_verify_hypersense_token(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch verify_hypersense_token")
            )
            .add_route(
                Route{"signout"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_signout_hypersense_token(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch signout_hypersense_token")
            );
    }

    explicit HypersenseRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{&context},
        m_runtime{context},
        m_policy{},
        m_request_support{m_policy},
        m_input_support{context}
    {
    }

private:
    ::app::context::AppContext* m_context{nullptr};
    HypersenseRuntime m_runtime;
    HypersensePolicy m_policy;
    core::behavior::RequestSupport<HypersensePolicy> m_request_support;
    HypersenseInputSupport m_input_support;
};

} // namespace app::handlers::hypersense
