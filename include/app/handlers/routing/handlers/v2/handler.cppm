export module app_handlers_routing:handlers_v2;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v2_algorithms;
import :handlers_v2_decision;

export namespace app::handlers::routing {

/**
 * v2 version facade. Composes family partitions and replays the exact original
 * add_route order (routes 30-38). Runtime and input support are owned once by
 * the root facade; children hold refs.
 */
class RoutingHandlersV2
{
public:
    RoutingHandlersV2(
        app::context::AppContext& context,
        RoutingRuntime& runtime,
        RoutingInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_algorithms{context, runtime, input_support},
        m_decision{context, runtime, input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_algorithms.install_route_30(router);
        m_algorithms.install_route_31(router);
        m_algorithms.install_route_32(router);
        m_algorithms.install_route_33(router);
        m_algorithms.install_route_34(router);
        m_algorithms.install_route_35(router);
        m_algorithms.install_route_36(router);
        m_decision.install_route_37(router);
        m_decision.install_route_38(router);
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingV2AlgorithmsHandlers m_algorithms;
    RoutingV2DecisionHandlers m_decision;
};

} // namespace app::handlers::routing
