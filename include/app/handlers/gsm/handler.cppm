export module app_handlers_gsm:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1;
import :handlers_v2;

export namespace app::handlers::gsm {

/**
 * Handler of handlers: owns the single shared runtime and input helper, then
 * delegates registration to the version partitions. Public shape is unchanged
 * (class name, ctor, install), so all.cppm / routes.cppm need no edits.
 */
class GsmRouteHandlers
{
public:
    explicit GsmRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
        m_v2.install(router);
    }

private:
    GsmRuntime m_runtime;            // SINGLE runtime instance — shared mutable state.
    GsmInputSupport m_input_support; // SINGLE input helper.
    GsmHandlersV1 m_v1;              // holds AppContext&, GsmRuntime&, GsmInputSupport&.
    GsmHandlersV2 m_v2;
};

} // namespace app::handlers::gsm
