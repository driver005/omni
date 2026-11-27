export module app_routing:offer_engine_router;

import std;
import interfaces;
import core_router;
import core_behavior;
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

import :offer_engine_input;
import :offer_engine_result;
import :offer_engine_runtime;

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
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream: &auth::AdminApiAuth (routes/offer_engine.rs:23). The registry entry
        // `offer_engine_connectivity_check` carries the equivalent platform-admin rule
        // (auth/registry.cppm:244) and `authorize_route` enforces it.
        if (const auto auth_error =
                m_request_support.authorize(req, "offer_engine_connectivity_check", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        m_runtime.connectivity_check(m_request_support.context(req)).handle(sink);
    }

    /** @brief Port of `routes::offer_engine::offer_engine_browse_offers` (routes:29-53). */
    void handle_offer_engine_browse_offers(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream: HeaderAuth(ApiKeyAuth { allow_connected_scope_operation: true,
        // allow_platform_self_operation: false }) (routes/offer_engine.rs:46-49).
        // The registry entry `offer_engine_browse_offers` carries those flags
        // (auth/registry.cppm:243).
        if (const auto auth_error =
                m_request_support.authorize(req, "offer_engine_browse_offers", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        auto parsed =
            core::behavior::Serializer<::app::dto::offer_engine::BrowseOffersRequest>::pass(
                req,
                sink
            );
        if (!parsed) {
            return;
        }

        // Upstream `browse_offers(state, auth.platform, profile_id, payload)`
        // (browse.rs:22-28): dimensions come from the authenticated CallContext.
        m_runtime.browse_offers(*parsed, m_request_support.context(req)).handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
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

    explicit OfferEngineRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy}
    {
    }

private:
    OfferEngineRuntime m_runtime;
    OfferEngineInputSupport m_input_support;
    OfferEnginePolicy m_policy;
    OfferEngineRequestSupport m_request_support;
};

} // namespace app::handlers::offer_engine
