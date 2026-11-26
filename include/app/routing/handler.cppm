export module app_routing:routing_area_handler;

import std;
import core_router;
import app_context;

import :blocklist_base;
import :feature_matrix_base;
import :gsm_base;
import :offer_engine_base;
import :routing_base;
import :superposition_proxy_base;
import :superposition_sdk_config_base;

export namespace app::api {

/**
 * Area facade. Owns the seven domain facades and replays the original global
 * AppRoutes order for this area: blocklist, feature_matrix, gsm, offer_engine,
 * routing, superposition_proxy, superposition_sdk_config.
 */
class RoutingHandlers
{
public:
    explicit RoutingHandlers(::app::context::AppContext& ctx) noexcept :
        m_context{ctx},
        m_blocklist{ctx},
        m_feature_matrix{ctx},
        m_gsm{ctx},
        m_offer_engine{ctx},
        m_routing{ctx},
        m_superposition_proxy{ctx},
        m_superposition_sdk_config{ctx}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_blocklist.install_routes(router);
        m_feature_matrix.install_routes(router);
        m_gsm.install_routes(router);
        m_offer_engine.install_routes(router);
        m_routing.install_routes(router);
        m_superposition_proxy.install_routes(router);
        m_superposition_sdk_config.install_routes(router);
    }

private:
    ::app::context::AppContext& m_context;
    ::app::handlers::blocklist::BlocklistRouteHandlers m_blocklist;
    ::app::handlers::feature_matrix::FeatureMatrixRouteHandlers m_feature_matrix;
    ::app::handlers::gsm::GsmRouteHandlers m_gsm;
    ::app::handlers::offer_engine::OfferEngineRouteHandlers m_offer_engine;
    ::app::handlers::routing::RoutingRouteHandlers m_routing;
    ::app::handlers::superposition_proxy::SuperpositionProxyRouteHandlers m_superposition_proxy;
    ::app::handlers::superposition_sdk_config::SuperpositionSdkConfigRouteHandlers
        m_superposition_sdk_config;
};

} // namespace app::api
