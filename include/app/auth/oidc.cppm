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
        if (const auto auth_error = m_request_support.authorize(req, "oidc_discovery", {})) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        OidcDiscoveryInput input{};
        auto result = m_runtime.discovery(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        auto encoded = result->serialize(discovery_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    void handle_jwks_endpoint(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(req, "jwks_endpoint", {})) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        JwksEndpointInput input{};
        auto result = m_runtime.jwks(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        auto encoded = result->serialize(jwks_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    void handle_oidc_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(req, "oidc_authorize", {})) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        // Upstream uses web::Query<OidcAuthorizeQuery> exclusively.
        const auto payload = m_input_support.parse_authorize_query(req);
        if (!payload) {
            const auto error = oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidRequest);
            error.send(res, send);
            return;
        }
        // The session identity is a pure request read, extracted here because the runtime
        // never touches IRequest. Missing session becomes AccessDenied inside the runtime.
        const auto user_id = m_input_support.authenticated_user_id(req);

        OidcAuthorizeInput input{*payload, user_id};
        auto result = m_runtime.authorize(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::FOUND);
        res.set_header(interfaces::io::types::Token::LOCATION, result->second);
        res.set_body(serde::Ser::serialize("application/json", result->first));
        send();
    }

    void handle_oidc_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream applies OIDC_CLIENT_AUTH (HTTP Basic), not a dashboard JWT: no
        // authorize_operation call. The exact Basic validation lives in the runtime.

        const auto request = m_input_support.parse_token_request(req);
        if (!request) {
            const auto error = oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidRequest);
            error.send(res, send);
            return;
        }
        const auto basic = m_input_support.parse_basic_auth(req);
        if (!basic) {
            const auto error = oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidBasicAuth);
            error.send(res, send);
            return;
        }

        OidcTokenInput input{*request, *basic};
        auto result = m_runtime.token(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        auto encoded = result->serialize(token_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
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
