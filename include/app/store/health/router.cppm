export module app_store:health_router;

import std;
import interfaces;
import core_router;
import core_behavior;
import app_context;

import :health_input;
import :health_runtime;
import :health_handlers_v1;
import :health_handlers_v2;

export namespace app::handlers::health {

/**
 * Local substitutes for the upstream Cargo features that select the health
 * scope. This port has no Cargo `#[cfg]`; both v1 and v2 are compiled in here,
 * matching an upstream release build.
 */
inline constexpr bool kV1 = true; // /health scope
inline constexpr bool kV2 = true; // /v2/health scope

/**
 * Handler of handlers: owns the single shared runtime and input helper, then
 * delegates registration to the version partitions. Public shape is unchanged
 * (class name, ctor, install), so all.cppm / routes.cppm need no edits.
 */
class HealthRouteHandlers
{
public:
    explicit HealthRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy},
        m_v1{context, m_runtime, m_request_support},
        m_v2{context, m_runtime, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        if constexpr (kV1) {
            m_v1.install(router);
        }
        if constexpr (kV2) {
            m_v2.install(router);
        }
    }

private:
    HealthRuntime m_runtime;            // SINGLE runtime instance — shared mutable state.
    HealthInputSupport m_input_support; // SINGLE input helper.
    HealthPolicy m_policy;              // SINGLE auth policy.
    core::behavior::RequestSupport<HealthPolicy> m_request_support;
    HealthHandlersV1 m_v1; // holds AppContext&, HealthRuntime&, RequestSupport<HealthPolicy>&.
    HealthHandlersV2 m_v2;
};

} // namespace app::handlers::health
