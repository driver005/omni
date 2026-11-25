export module app_handlers_configs:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1;
import :handlers_v2;

export namespace app::handlers::configs {

/**
 * Local substitutes for the upstream Cargo features that select the config
 * scope and the admin auth type. This port has no Cargo `#[cfg]`, so these
 * constexpr gates are the exact local equivalent: upstream registers exactly
 * one scope per build (routes/app.rs:2412-2424 v1 / :2427-2439 v2) and selects
 * `AdminApiAuth` vs `V2AdminApiAuth` at compile time (routes/configs.rs:11-14).
 * Both feature sets are compiled in here, matching an upstream release build
 * with v1 and v2 enabled. Both admin auth variants resolve to the same
 * admin-level rule in the shared registry (app/auth/registry.cppm:59-62).
 */
inline constexpr bool kV1 = true; // /configs
inline constexpr bool kV2 = true; // /v2/configs

/**
 * Handler of handlers: owns the single shared runtime and input helper, then
 * delegates registration to the version partitions. Public shape is unchanged
 * (class name, ctor, install), so all.cppm / routes.cppm need no edits.
 */
class ConfigsRouteHandlers
{
public:
    explicit ConfigsRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        if constexpr (kV1) {
            m_v1.install(router);
        }
        if constexpr (kV2) {
            m_v2.install(router);
        }
    }

private:
    ConfigsRuntime m_runtime;            // SINGLE runtime instance — shared mutable state.
    ConfigsInputSupport m_input_support; // SINGLE input helper.
    ConfigsHandlersV1 m_v1; // holds AppContext&, ConfigsRuntime&, ConfigsInputSupport&.
    ConfigsHandlersV2 m_v2;
};

} // namespace app::handlers::configs
