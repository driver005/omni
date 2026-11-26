export module app_payment:revenue_recovery_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :revenue_recovery_input;
import :revenue_recovery_result;
import :revenue_recovery_runtime;
import :revenue_recovery_handlers_v2;

export namespace app::handlers::revenue_recovery {

/**
 * Facade for the revenue-recovery route family. Owns the single shared runtime;
 * version partitions receive it by reference and only add their own routes.
 */
class RevenueRecoveryRouteHandlers
{
public:
    explicit RevenueRecoveryRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_v2{context, m_runtime}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryRuntime m_runtime;
    RevenueRecoveryHandlersV2 m_v2;
};

} // namespace app::handlers::revenue_recovery
