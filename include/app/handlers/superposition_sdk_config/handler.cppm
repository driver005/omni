export module app_handlers_superposition_sdk_config:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1;

export namespace app::handlers::superposition_sdk_config {

/**
 * @brief Facade ("handler of handlers").
 *
 * Owns the ONE shared `SuperpositionSdkConfigRuntime` and the single input
 * support object; the v1 partition holds references to them, never its own
 * instances. Both upstream routes are v1 (`app.rs:3584-3595`), so there is no
 * v2 partition. The public class name, ctor `(AppContext&)` and `install(router)`
 * are unchanged, so all.cppm/routes.cppm need no edits. Business logic,
 * input.cppm, result.cppm and runtime/** stay untouched.
 */
class SuperpositionSdkConfigRouteHandlers
{
public:
    explicit SuperpositionSdkConfigRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v1.install_v1(router);
    }

private:
    SuperpositionSdkConfigRuntime m_runtime;            // SINGLE runtime - no duplicated state.
    SuperpositionSdkConfigInputSupport m_input_support; // SINGLE parse/auth support.
    SuperpositionSdkConfigHandlersV1 m_v1;              // Holds AppContext& + refs above.
};

} // namespace app::handlers::superposition_sdk_config
