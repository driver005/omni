export module app_store:currency_router;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
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
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // server_wrap auth_type(ApiKeyAuth, DashboardNoPermissionAuth) OR-path.
        if (const auto auth_error = m_request_support.authorize(req, "retrieve_forex", {})) {
            auth_error->send(sink);
            return;
        }

        // ApplicationResponse::Json(FxExchangeRatesCacheEntry): typed document.
        m_runtime.retrieve_forex(m_request_support.context(req)).handle(sink);
    }

    // ── routes/currency.rs:36-72 — convert_forex ──────────────────────────────
    void handle_convert_forex(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_request_support.authorize(req, "convert_forex", {})) {
            auth_error->send(sink);
            return;
        }

        // web::Query<CurrencyConversionParams> (routes/currency.rs:39).
        auto params = m_input_support.parse_conversion_params(req);
        if (!params) {
            params.error().send(sink);
            return;
        }

        // ApplicationResponse::Json(CurrencyConversionResponse) (:34-36).
        m_runtime.convert_forex(*params, m_request_support.context(req)).handle(sink);
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
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy}
    {
    }

private:
    CurrencyRuntime m_runtime;
    CurrencyInputSupport m_input_support;
    CurrencyPolicy m_policy;
    core::behavior::RequestSupport<CurrencyPolicy> m_request_support;
};

} // namespace app::handlers::currency
