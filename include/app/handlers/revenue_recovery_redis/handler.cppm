export module app_handlers_revenue_recovery_redis:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v2;

export namespace app::handlers::revenue_recovery_redis {

/**
 * Facade for the revenue-recovery Redis route family. Owns the single shared runtime;
 * version partitions receive it by reference and only add their own routes.
 */
class RevenueRecoveryRedisRouteHandlers
{
public:
    explicit RevenueRecoveryRedisRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_v2{context, m_runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    app::context::AppContext& m_context;
    RevenueRecoveryRedisRuntime m_runtime;
    RevenueRecoveryRedisHandlersV2 m_v2;
};

} // namespace app::handlers::revenue_recovery_redis
