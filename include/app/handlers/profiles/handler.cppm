export module app_handlers_profiles:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1;
import :handlers_v2;

export namespace app::handlers::profiles {

/**
 * Strict port of upstream `routes/profiles.rs` + the profile functions of
 * `core/admin.rs`. Upstream external infrastructure (accounts_store, master
 * key, key store, decision engine, outbound destination resolver) is replaced
 * by in-process AppContext facilities; every guard, error and wire shape
 * otherwise follows the upstream source.
 *
 * Handler of handlers: owns the single shared runtime and input helper, then
 * delegates registration to the version partitions. Public shape is unchanged
 * (class name, ctor, install), so all.cppm / routes.cppm need no edits.
 */
class ProfilesRouteHandlers
{
public:
    explicit ProfilesRouteHandlers(app::context::AppContext& context) noexcept :
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
    ProfilesRuntime m_runtime;            // SINGLE runtime instance — shared mutable state.
    ProfilesInputSupport m_input_support; // SINGLE input helper.
    ProfilesHandlersV1 m_v1; // holds AppContext&, ProfilesRuntime&, ProfilesInputSupport&.
    ProfilesHandlersV2 m_v2;
};

} // namespace app::handlers::profiles
