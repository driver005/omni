export module app_auth:external_service_auth_handler;

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

import :external_service_auth_input;
import :external_service_auth_result;
import :external_service_auth_runtime;

export namespace app::handlers::external_service_auth {

class ExternalServiceAuthRouteHandlers
{
public:
    explicit ExternalServiceAuthRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{&context},
        m_runtime{context},
        m_policy{context},
        m_request_support{m_policy}
    {
    }

    /** @brief routes/external_service_auth.rs:11-27 — validate_token. */
    void handle_validate_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "validate_token", {})) {
            auth_error->send(sink);
            return;
        }

        // The typed DTO's `ValidatingService` FieldConverter accepts the snake_case
        // wire value "offer_engine" and rejects anything else, matching upstream
        // `#[serde(rename_all = "snake_case")]` (api_models:18-29).
        using Dto = ::app::dto::external_service_auth::ValidateTokenRequest;
        auto input = core::behavior::Serializer<Dto>::pass(req, sink);
        if (!input) {
            return;
        }
        m_runtime.validate_token(*input, m_request_support.context(req)).handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream mounts exactly POST /external-service/validate-token
        // (app.rs:2004-2009; routes/external_service_auth.rs:11-27). No other
        // route exists in this domain upstream; the old C++-only prefixed alias
        // was removed for strict parity.
        Router{router, "external-service"}.add_route(
            Route{"validate-token"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_validate_token(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch validate_token (upstream path)")
        );
    }

private:
    ::app::context::AppContext* m_context{nullptr};
    ExternalServiceAuthRuntime m_runtime;
    ExternalServiceAuthPolicy m_policy;
    core::behavior::RequestSupport<ExternalServiceAuthPolicy> m_request_support;
};

} // namespace app::handlers::external_service_auth
