export module app_handlers_revenue_recovery_redis:runtime_get_revenue_recovery_redis_data;

import std;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::revenue_recovery_redis {

class GetRevenueRecoveryRedisDataRuntime
{
public:
    explicit GetRevenueRecoveryRedisDataRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetRevenueRecoveryRedisDataResult, core::error::AppError>
    get_revenue_recovery_redis_data(const GetRevenueRecoveryRedisDataInput& input) const
    {
        // Upstream key construction (revenue_recovery_redis_operation.rs:139-145,
        // :1086-1089): Status -> customer:{id}:status, Tokens -> customer:{id}:tokens.
        // The in-process cache is the local substitute for the Redis instance; the
        // natural Redis key is used verbatim, namespaced so it cannot collide with
        // unrelated cache entries.
        const auto redis_key = std::string{"customer:"} + input.connector_customer_id +
                               (input.is_tokens ? ":tokens" : ":status");
        const auto cache_key = std::string{"revenue_recovery_redis.redis:"} + redis_key;

        // Upstream only errors when the Redis connection / get_ttl fails
        // (revenue_recovery_redis_operation.rs:1078-1098). A closed local cache is
        // the in-process equivalent of that infrastructure failure. A key that is
        // simply absent is NOT an error upstream: it yields a 200 response with
        // exists=false (see get_redis_key_data_raw above).
        if (!m_context.cache().is_open()) {
            return std::unexpected{m_support.not_found_error(input.connector_customer_id)};
        }

        const auto raw = m_support.get_redis_key_data_raw(
            cache_key,
            input.is_tokens,
            input.connector_customer_id
        );

        // Flow::RevenueRecoveryRedis instrumentation (revenue_recovery_redis.rs:11, :18).
        m_context.metrics().record_flow("RevenueRecoveryRedis");
        m_context.metrics()
            .counter("revenue_recovery_redis.get_revenue_recovery_redis_data")
            .increment();

        // Upstream core/revenue_recovery_data_backfill.rs:130-144:
        // Ok((exists, ttl_seconds, data)) -> RedisDataResponse -> ApplicationResponse::Json.
        using Response = app::dto::revenue_recovery_data_backfill::RedisDataResponse;
        Response response;
        response.set_exists(raw.exists);
        response.set_ttl_seconds(raw.ttl_seconds);
        response.set_data(raw.data);
        return GetRevenueRecoveryRedisDataResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    RevenueRecoveryRedisRuntimeSupport m_support;
};

} // namespace app::handlers::revenue_recovery_redis
