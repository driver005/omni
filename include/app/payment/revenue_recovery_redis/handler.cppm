export module app_payment:revenue_recovery_redis_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :revenue_recovery_redis_input;
import :revenue_recovery_redis_result;
import :revenue_recovery_redis_runtime;
import :revenue_recovery_redis_handlers_v2;

export namespace app::handlers::revenue_recovery_redis {

/**
 * Facade for the revenue-recovery Redis route family. Owns the single shared runtime;
 * version partitions receive it by reference and only add their own routes.
 */
class RevenueRecoveryRedisRouteHandlers
{
public:
    explicit RevenueRecoveryRedisRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_v2{context, m_runtime},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_v2.bind_request_support(m_request_support);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryRedisRuntime m_runtime;
    RevenueRecoveryRedisHandlersV2 m_v2;
    RevenueRecoveryRedisInputSupport m_input_support;
    RevenueRecoveryRedisPolicy m_policy;
    RevenueRecoveryRedisRequestSupport m_request_support;
};

} // namespace app::handlers::revenue_recovery_redis
