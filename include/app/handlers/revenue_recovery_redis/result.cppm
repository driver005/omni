export module app_handlers_revenue_recovery_redis:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::revenue_recovery_redis {

// The wire fields are inherited from the exact DTO the old handler built and
// encoded, so field names, order and wrapper types stay byte-identical. The
// Serializable specialization reuses that DTO's FieldDesc tuple; those
// descriptors apply to the derived object just as they do to the base object.
class GetRevenueRecoveryRedisDataResult :
    public app::dto::revenue_recovery_data_backfill::RedisDataResponse
{
public:
    GetRevenueRecoveryRedisDataResult() = default;

    explicit GetRevenueRecoveryRedisDataResult(
        app::dto::revenue_recovery_data_backfill::RedisDataResponse value
    ) noexcept :
        RedisDataResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::revenue_recovery_redis

template<>
struct serde::Serializable<app::handlers::revenue_recovery_redis::GetRevenueRecoveryRedisDataResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::revenue_recovery_data_backfill::RedisDataResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "get_revenue_recovery_redis_data_result";
    }
};
