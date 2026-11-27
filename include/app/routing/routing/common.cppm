export module app_routing:routing_handlers_common;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :routing_input;
import :routing_result;
import :routing_runtime;
import :routing_handlers_dynamic_routing;
import :routing_handlers_volume_split;

export namespace app::handlers::routing {

/**
 * Unversioned (common) facade. Composes the dynamic-routing and volume-split
 * families and replays the exact original add_route order (routes 39-43),
 * which the original install() registered after the v2 routes.
 * Runtime and input support are owned once by the root facade; children hold refs.
 */
class RoutingHandlersCommon
{
public:
    RoutingHandlersCommon(
        ::app::context::AppContext& context,
        RoutingRuntime& runtime,
        RoutingInputSupport& input_support,
        RoutingRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support},
        m_dynamic_routing{context, runtime, input_support, request_support},
        m_volume_split{context, runtime, input_support, request_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_dynamic_routing.install_route_39(router);
        m_volume_split.install_route_40(router);
        m_volume_split.install_route_41(router);
        m_dynamic_routing.install_route_42(router);
        m_dynamic_routing.install_route_43(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
    RoutingDynamicRoutingHandlers m_dynamic_routing;
    RoutingVolumeSplitHandlers m_volume_split;
};

} // namespace app::handlers::routing
