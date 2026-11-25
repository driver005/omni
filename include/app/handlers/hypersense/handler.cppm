export module app_handlers_hypersense:handler;

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
import app_services;

import :input;
import :result;
import :runtime;

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
        GetTokenInput input{m_input_support.bearer_token(req), m_input_support.merchant_id(req)};
        auto result = m_runtime.get_token(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
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
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::external_service_auth::ExternalVerifyTokenRequest;
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
        VerifyTokenInput input{*parsed};
        auto result = m_runtime.verify_token(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
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
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::external_service_auth::ExternalSignoutTokenRequest;
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
        SignoutTokenInput input{*parsed};
        auto result = m_runtime.signout_token(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::vector<std::byte>{});
        send();
    }

    void install(core::router::RouterContext<>& router)
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

    explicit HypersenseRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{&context},
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    app::context::AppContext* m_context{nullptr};
    HypersenseRuntime m_runtime;
    HypersenseInputSupport m_input_support;
};

} // namespace app::handlers::hypersense
