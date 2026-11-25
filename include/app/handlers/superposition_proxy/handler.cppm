export module app_handlers_superposition_proxy:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1;

export namespace app::handlers::superposition_proxy {

/**
 * @brief Facade ("handler of handlers").
 *
 * Owns the ONE shared `SuperpositionProxyRuntime` and the single input support
 * object; the v1 partition holds references to them, never its own instances.
 * All nine upstream routes are v1 (`v1/superposition/...`), so there is no v2
 * partition. Public class name, ctor `(AppContext&)` and `install(router)` are
 * unchanged: all.cppm/routes.cppm need no edits. Business logic, input.cppm,
 * result.cppm and runtime/** stay untouched.
 */
class SuperpositionProxyRouteHandlers
{
public:
    explicit SuperpositionProxyRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{&context},
        m_input_support{context},
        m_runtime{context},
        m_v1{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
    }

private:
    app::context::AppContext* m_context{nullptr};
    SuperpositionProxyInputSupport m_input_support; // SINGLE parse/auth support.
    SuperpositionProxyRuntime m_runtime;            // SINGLE runtime - no duplicated state.
    SuperpositionProxyHandlersV1 m_v1;              // Holds AppContext& + refs above.
};

} // namespace app::handlers::superposition_proxy
