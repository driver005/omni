export module app_payment:revenue_recovery_redis_runtime;
import core_behavior;

import std;
import core_error;
import app_context;

import :revenue_recovery_redis_input;
import :revenue_recovery_redis_result;
import :revenue_recovery_redis_runtime_common;
import :revenue_recovery_redis_runtime_get_revenue_recovery_redis_data;

export namespace app::handlers::revenue_recovery_redis {

class RevenueRecoveryRedisRuntime
{
public:
    explicit RevenueRecoveryRedisRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_revenue_recovery_redis_data{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::revenue_recovery_data_backfill::RedisDataResponse>
    get_revenue_recovery_redis_data(const core::behavior::CallContext& ctx) const
    {
        return m_get_revenue_recovery_redis_data.get_revenue_recovery_redis_data(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    GetRevenueRecoveryRedisDataRuntime m_get_revenue_recovery_redis_data;
};

} // namespace app::handlers::revenue_recovery_redis
