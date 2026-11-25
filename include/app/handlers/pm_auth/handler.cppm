export module app_handlers_pm_auth:handler;

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

export namespace app::handlers::pm_auth {

class PmAuthRouteHandlers
{
public:
    void handle_link_token_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const std::string body = req.body_as_string();
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

        // Upstream uses SDK / client-secret API-key auth (routes/pm_auth.rs:20-45,86-111).
        // Base rule comes from RouteAuthRegistry; SDK and client-secret schemes are accepted
        // as alternates so both v1 SDK calls and v2 client-secret calls authenticate.
        const auto auth = m_input_support.authorize_request(req, body, "link_token_create");
        if (auth.error) {
            const auto& error = *auth.error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto merchant = m_input_support.merchant_from_request(req);
        if (!merchant) {
            const auto& error = merchant.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto normalized = m_input_support.normalize_link_token_body(body);
        if (!normalized) {
            const auto& error = normalized.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::pm_auth::LinkTokenCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", *normalized);
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
        // routes/pm_auth.rs:62-64 — auth.client_secret overwrites payload.client_secret.
        if (const auto authenticated_secret = auth.context.claim(app::auth::kClaimClientSecret);
            authenticated_secret.has_value() && !authenticated_secret->empty()) {
            parsed->set_client_secret(std::string{*authenticated_secret});
        }
        const auto payment_id = parsed->get_payment_id().get_value();
        const auto payment_method = parsed->get_payment_method().get_value();
        const auto payment_method_type = parsed->get_payment_method_type().get_value();
        if (payment_id.empty() || payment_method.empty() || payment_method_type.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "payment_id, payment_method and payment_method_type are required"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto client_platform = app::http::HeaderReader{}.get(req, "x-client-platform");
        const auto app_id = app::http::HeaderReader{}.get(req, "x-app-id");
        const auto redirect_uri = app::http::HeaderReader{}.get(req, "x-redirect-uri");
        LinkTokenCreateInput input{*parsed, *merchant, auth, client_platform, app_id, redirect_uri};
        auto result = m_runtime.link_token_create(input);
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

    void handle_exchange_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const std::string body = req.body_as_string();
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

        const auto auth = m_input_support.authorize_request(req, body, "exchange_token");
        if (auth.error) {
            const auto& error = *auth.error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto merchant = m_input_support.merchant_from_request(req);
        if (!merchant) {
            const auto& error = merchant.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto normalized = m_input_support.normalize_exchange_body(body);
        if (!normalized) {
            const auto& error = normalized.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        using Input = app::dto::pm_auth::ExchangeTokenCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", *normalized);
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
        // routes/pm_auth.rs:119-121 — auth.client_secret overwrites payload.client_secret.
        if (const auto authenticated_secret = auth.context.claim(app::auth::kClaimClientSecret);
            authenticated_secret.has_value() && !authenticated_secret->empty()) {
            parsed->set_client_secret(std::string{*authenticated_secret});
        }
        const auto public_token = parsed->get_public_token();
        const auto payment_id = parsed->get_payment_id().get_value();
        const auto payment_method = parsed->get_payment_method().get_value();
        const auto payment_method_type = parsed->get_payment_method_type().get_value();
        if (public_token.empty() || payment_id.empty() || payment_method.empty() ||
            payment_method_type.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "public_token, payment_id, payment_method and payment_method_type are required"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        ExchangeTokenInput input{*parsed, *merchant, auth, body};
        auto result = m_runtime.exchange_token(input);
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

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payment_methods"}
            .add_router(
                Router{router, "auth"}.add_route(
                    Route{"link"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_link_token_create(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch link_token_create")
                )
            )
            .add_router(
                Router{router, "auth"}.add_route(
                    Route{"exchange"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_exchange_token(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch exchange_token")
                )
            );
    }

    explicit PmAuthRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    PmAuthRuntime m_runtime;
    PmAuthInputSupport m_input_support;
};

} // namespace app::handlers::pm_auth
