export module app_store:currency_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_context_core;
import app_http;
import core_error;
import app_metrics;

import :currency_input;
import :currency_result;
import :currency_runtime;

export namespace app::handlers::currency {

class CurrencyRouteHandlers
{
public:
    // ── routes/currency.rs:11-33 — retrieve_forex ─────────────────────────────
    void handle_retrieve_forex(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // server_wrap auth_type(ApiKeyAuth, DashboardNoPermissionAuth) OR-path.
        const auto auth = m_input_support.authorize_forex(req);
        if (auth.error) {
            const auto& error = *auth.error;
            error.send(res, send);
            return;
        }

        RetrieveForexInput input{auth};
        auto result = m_runtime.retrieve_forex(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // ApplicationResponse::Json(FxExchangeRatesCacheEntry): typed document.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // ── routes/currency.rs:36-72 — convert_forex ──────────────────────────────
    void handle_convert_forex(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = m_input_support.authorize_forex(req);
        if (auth.error) {
            const auto& error = *auth.error;
            error.send(res, send);
            return;
        }

        // web::Query<CurrencyConversionParams> (routes/currency.rs:39).
        auto params = m_input_support.parse_conversion_params(req);
        if (!params) {
            const auto& error = params.error();
            error.send(res, send);
            return;
        }

        ConvertForexInput input{auth, *params};
        auto result = m_runtime.convert_forex(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // ApplicationResponse::Json(CurrencyConversionResponse) (:34-36).
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "forex"}
            .add_route(
                Route{"rates"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_retrieve_forex(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch retrieve_forex")
            )
            .add_route(
                Route{"convert_from_minor"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_convert_forex(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch convert_forex")
            );
    }

    explicit CurrencyRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    CurrencyRuntime m_runtime;
    CurrencyInputSupport m_input_support;
};

} // namespace app::handlers::currency
