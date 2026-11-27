export module app_auth:oidc_handler;

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
import app_keystore;
import app_services;

import :oidc_input;
import :oidc_result;
import :oidc_runtime;

export namespace app::handlers::oidc {

class OidcRouteHandlers
{
public:
    void handle_oidc_discovery(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "oidc_discovery", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        m_runtime.discovery(m_request_support.context(req)).handle(sink);
    }

    void handle_jwks_endpoint(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "jwks_endpoint", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        m_runtime.jwks(m_request_support.context(req)).handle(sink);
    }

    void handle_oidc_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "oidc_authorize", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        // Upstream uses web::Query<OidcAuthorizeQuery> exclusively.
        const auto payload = m_input_support.parse_authorize_query(req);
        if (!payload) {
            const auto error = oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidRequest);
            error.send(sink);
            return;
        }
        // Session identity and Basic credentials are captured in CallContext; runtime
        // never touches IRequest. Missing session becomes AccessDenied inside runtime.
        m_runtime.authorize(*payload, m_request_support.context(req)).handle(sink);
    }

    void handle_oidc_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream applies OIDC_CLIENT_AUTH (HTTP Basic), not a dashboard JWT: no
        // authorize_operation call. The exact Basic validation lives in the runtime.

        const auto request = m_input_support.parse_token_request(req);
        if (!request) {
            const auto error = oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidRequest);
            error.send(sink);
            return;
        }
        if (!m_input_support.parse_basic_auth(req)) {
            const auto error = oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidBasicAuth);
            error.send(sink);
            return;
        }

        m_runtime.token(*request, m_request_support.context(req)).handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream mounts these at the root via web::scope(""). Single-segment
        // router/route literals keep the trie resolution exact.
        Router{router, ".well-known"}.add_route(
            Route{"openid-configuration"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_oidc_discovery(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch oidc_discovery")
        );
        Router{router, "oauth2"}
            .add_route(
                Route{"jwks"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_jwks_endpoint(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch jwks_endpoint")
            )
            .add_route(
                Route{"token"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_oidc_token(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch oidc_token")
            );
        Router{router, "oidc"}.add_route(
            Route{"authorize"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_oidc_authorize(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch oidc_authorize")
        );
    }

    explicit OidcRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_policy{context},
        m_request_support{m_policy},
        m_input_support{context}
    {
    }

private:
    OidcRuntime m_runtime;
    OidcPolicy m_policy;
    core::behavior::RequestSupport<OidcPolicy> m_request_support;
    OidcInputSupport m_input_support;
};

} // namespace app::handlers::oidc
