export module app_routing:routing_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :routing_input;
import :routing_result;
import :routing_runtime;
import :routing_handlers_v1;
import :routing_handlers_v2;
import :routing_handlers_common;

export namespace app::handlers::routing {

/**
 * Handler of handlers. Public shape (class name, ctor(AppContext&), install) is
 * unchanged, so all.cppm / routes.cppm need no edits. The single RoutingRuntime
 * and RoutingInputSupport are owned here; version/common children hold refs, so
 * shared mutable state is never duplicated.
 *
 * install() replays the original route order: v1 routes 1-29 (including the
 * payouts scope), then v2 routes 30-38, then unversioned routes 39-43.
 */
class RoutingRouteHandlers
{
public:
    explicit RoutingRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support},
        m_common{context, m_runtime, m_input_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
        m_v2.install(router);
        m_common.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime m_runtime;            // SINGLE runtime instance.
    RoutingInputSupport m_input_support; // SINGLE input helper.
    RoutingHandlersV1 m_v1;              // holds AppContext& + Runtime& + InputSupport&.
    RoutingHandlersV2 m_v2;
    RoutingHandlersCommon m_common;
};

} // namespace app::handlers::routing
