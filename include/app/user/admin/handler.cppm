export module app_user:admin_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :admin_input;
import :admin_result;
import :admin_runtime;
import :admin_handlers_v1;
import :admin_handlers_v2;

export namespace app::handlers::admin {

/**
 * Upstream splits the admin surface across two mutually-exclusive Cargo
 * features (`v1` / `v2`, e.g. routes/admin.rs:11-16 vs :30-37 and
 * routes/app.rs:2112-2131 vs :2133-2151). This port builds both, so these
 * constexpr gates are the exact local equivalent of the cfg attributes.
 */
inline constexpr bool kV1 = true;
inline constexpr bool kV2 = true;

class AdminRouteHandlers
{
public:
    explicit AdminRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support}
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
    AdminRuntime m_runtime;            // SINGLE runtime instance, shared by every family.
    AdminInputSupport m_input_support; // SINGLE input helper, shared by every family.
    AdminHandlersV1 m_v1;
    AdminHandlersV2 m_v2;
};

} // namespace app::handlers::admin
