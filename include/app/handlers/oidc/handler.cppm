export module app_handlers_oidc:handler;

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
import app_keystore;
import app_services;

import :input;
import :result;
import :runtime;

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
        if (const auto auth_error = m_input_support.authorize_operation(req, "oidc_discovery")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        OidcDiscoveryInput input{};
        auto result = m_runtime.discovery(input);
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

    void handle_jwks_endpoint(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize_operation(req, "jwks_endpoint")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        JwksEndpointInput input{};
        auto result = m_runtime.jwks(input);
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

    void handle_oidc_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize_operation(req, "oidc_authorize")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // Upstream uses web::Query<OidcAuthorizeQuery> exclusively.
        const auto payload = m_input_support.parse_authorize_query(req);
        if (!payload) {
            const auto error = oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidRequest);
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        // The session identity is a pure request read, extracted here because the runtime
        // never touches IRequest. Missing session becomes AccessDenied inside the runtime.
        const auto user_id = m_input_support.authenticated_user_id(req);

        OidcAuthorizeInput input{*payload, user_id};
        auto result = m_runtime.authorize(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (result->is_error()) {
            const auto error = oidc_wire::oidc_error(result->get_error_kind());
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        res.set_status(interfaces::io::types::Status::FOUND);
        res.set_header(interfaces::io::types::Token::LOCATION, result->get_location());
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
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
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto basic = m_input_support.parse_basic_auth(req);
        if (!basic) {
            const auto error = oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidBasicAuth);
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        OidcTokenInput input{*request, *basic};
        auto result = m_runtime.token(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        if (result->is_error()) {
            const auto error = oidc_wire::oidc_error(result->get_error_kind());
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void install(core::router::RouterContext<>& router)
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

    explicit OidcRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    OidcRuntime m_runtime;
    OidcInputSupport m_input_support;
};

} // namespace app::handlers::oidc
