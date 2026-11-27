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
        interfaces::io::ResponseSink sink{res, std::move(send)};
        m_runtime.get_token(m_request_support.context(req)).handle(sink);
    }

    // routes/hypersense.rs:75 — NoAuth. The payload token is the only credential.
    void handle_verify_hypersense_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        using Dto = ::app::dto::external_service_auth::ExternalVerifyTokenRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        m_runtime.verify_token(*input, m_request_support.context(req)).handle(sink);
    }

    // routes/hypersense.rs:51 — NoAuth. Successful core is bodyless StatusOk.
    void handle_signout_hypersense_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        using Dto = ::app::dto::external_service_auth::ExternalSignoutTokenRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        m_runtime.signout_token(*input, m_request_support.context(req)).handle(sink);
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
        m_request_support{m_policy}
    {
    }

private:
    ::app::context::AppContext* m_context{nullptr};
    HypersenseRuntime m_runtime;
    HypersensePolicy m_policy;
    core::behavior::RequestSupport<HypersensePolicy> m_request_support;
};

} // namespace app::handlers::hypersense
