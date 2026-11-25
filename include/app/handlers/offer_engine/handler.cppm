export module app_handlers_offer_engine:handler;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_connector;
import app_services;

import :input;
import :result;
import :runtime;

export namespace app::handlers::offer_engine {

class OfferEngineRouteHandlers
{
public:
    // ─────────────────────────── Routes ──────────────────────────────────────

    /** @brief Port of `routes::offer_engine::offer_engine_connectivity_check` (routes:11-27). */
    void handle_offer_engine_connectivity_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: &auth::AdminApiAuth (routes/offer_engine.rs:23). The registry entry
        // `offer_engine_connectivity_check` carries the equivalent platform-admin rule
        // (auth/registry.cppm:244) and `authorize_route` enforces it.
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error =
                m_input_support.authorize(auth, "offer_engine_connectivity_check")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        ConnectivityCheckInput input{auth};
        auto result = m_runtime.connectivity_check(input);
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

    /** @brief Port of `routes::offer_engine::offer_engine_browse_offers` (routes:29-53). */
    void handle_offer_engine_browse_offers(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: HeaderAuth(ApiKeyAuth { allow_connected_scope_operation: true,
        // allow_platform_self_operation: false }) (routes/offer_engine.rs:46-49).
        // The registry entry `offer_engine_browse_offers` carries those flags
        // (auth/registry.cppm:243).
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = m_input_support.authorize(auth, "offer_engine_browse_offers")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

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
        using Input = app::dto::offer_engine::BrowseOffersRequest;
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

        // Upstream `browse_offers(state, auth.platform, profile_id, payload)`
        // (browse.rs:22-28): dimensions come from the authenticated platform.
        OfferEngineDimensions dimensions{};
        dimensions.processor_merchant_id = auth.merchant_id();
        dimensions.organization_id = auth.org_id();
        if (!auth.profile_id().empty()) {
            dimensions.profile_id = auth.profile_id();
        }

        BrowseOffersInput input{*parsed, dimensions, auth};
        auto result = m_runtime.browse_offers(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // Both API error shapes (IR_22 / HE_00) arrive as AppError through the
        // `!result` branch above; the success case is the typed DTO below.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_response()));
        send();
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream scope `/offer_engine` with `/connectivity` and `/offers/list`
        // POST resources (routes/app.rs:838-847). No C++-only routes are installed.
        Router{router, "offer_engine"}
            .add_route(
                Route{"connectivity"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_offer_engine_connectivity_check(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch offer_engine_connectivity_check")
            )
            .add_router(
                Router{router, "offers"}.add_route(
                    Route{"list"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_offer_engine_browse_offers(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch offer_engine_browse_offers")
                )
            );
    }

    explicit OfferEngineRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    OfferEngineRuntime m_runtime;
    OfferEngineInputSupport m_input_support;
};

} // namespace app::handlers::offer_engine
