export module app_handlers_payout_link:handler;

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
import app_connector;
import app_http;
import app_metrics;
import app_store;
import app_services;

import :input;
import :result;
import :runtime;

export namespace app::handlers::payout_link {

class PayoutLinkRouteHandlers
{
public:
    // ─────────────────────────────────────────────────────────────────────────
    // routes/payout_link.rs:14-39 — render_payout_link
    // ─────────────────────────────────────────────────────────────────────────
    void handle_render_payout_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto path_ids = m_input_support.payout_link_ids_from_path(req);
        if (!path_ids.has_value()) {
            const auto& error = path_ids.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // &auth::MerchantIdAuth(merchant_id) (routes/payout_link.rs:35): bind the
        // principal to the path merchant id and reject a conflicting header.
        const auto denied =
            m_input_support.authorize_operation(req, "render_payout_link", path_ids->first);
        if (denied.has_value()) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // The old core read these headers straight off IRequest. Capture them
        // here; validate_payout_link_render_request_and_get_allowed_domains and
        // locale_from_request consume the values inside the runtime.
        const app::http::HeaderReader read{};
        PayoutLinkRenderInput input{
            path_ids->first,
            path_ids->second,
            read.get(req, "sec-fetch-dest"),
            read.get(req, "origin"),
            read.get(req, "referer"),
            read.get(req, "accept-language")
        };

        auto result = m_runtime.render_payout_link(input);
        if (!result.has_value()) {
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
        Router{router, "payout_link"}.add_router(
            Router{router, ":merchant_id"}.add_route(
                Route{":payout_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_render_payout_link(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch render_payout_link")
            )
        );
    }

    explicit PayoutLinkRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    PayoutLinkRuntime m_runtime;
    PayoutLinkInputSupport m_input_support;
};

} // namespace app::handlers::payout_link
