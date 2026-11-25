export module app_handlers_revenue_recovery_redis:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_get_revenue_recovery_redis_data;

export namespace app::handlers::revenue_recovery_redis {

class RevenueRecoveryRedisRuntime
{
public:
    explicit RevenueRecoveryRedisRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_revenue_recovery_redis_data{context}
    {
    }

    [[nodiscard]] std::expected<GetRevenueRecoveryRedisDataResult, core::error::AppError>
    get_revenue_recovery_redis_data(const GetRevenueRecoveryRedisDataInput& input) const
    {
        return m_get_revenue_recovery_redis_data.get_revenue_recovery_redis_data(input);
    }

private:
    app::context::AppContext& m_context;
    GetRevenueRecoveryRedisDataRuntime m_get_revenue_recovery_redis_data;
};

} // namespace app::handlers::revenue_recovery_redis
