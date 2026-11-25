export module app_handlers_revenue_recovery_data_backfill:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::revenue_recovery_data_backfill {

// The two processed-count responses inherit their wire fields from the exact DTO
// the old core built and encoded, so field names, order and wrapper types stay
// byte-identical. The Serializable specialization reuses the DTO's FieldDesc tuple.

class RevenueRecoveryDataBackfillResult :
    public app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse
{
public:
    RevenueRecoveryDataBackfillResult() = default;

    explicit RevenueRecoveryDataBackfillResult(
        app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse value
    ) noexcept :
        RevenueRecoveryDataBackfillResponse{std::move(value)}
    {
    }
};

class RetryStatsMigrationResult :
    public app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse
{
public:
    RetryStatsMigrationResult() = default;

    explicit RetryStatsMigrationResult(
        app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse value
    ) noexcept :
        RetryStatsMigrationResponse{std::move(value)}
    {
    }
};

// The rotated success bodies keep their exact DTO wire fields. Error paths are
// core::error::AppError values (with the upstream code in kind_name), serialized
// through the shared error branch of the handler.

class UpdateRedisDataResult :
    public app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse
{
public:
    UpdateRedisDataResult() = default;

    explicit UpdateRedisDataResult(
        app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse value
    ) noexcept :
        UpdateTokenStatusResponse{std::move(value)}
    {
    }
};

class UnlockStatusResult : public app::dto::revenue_recovery_data_backfill::UnlockStatusResponse
{
public:
    UnlockStatusResult() = default;

    explicit UnlockStatusResult(
        app::dto::revenue_recovery_data_backfill::UnlockStatusResponse value
    ) noexcept :
        UnlockStatusResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::revenue_recovery_data_backfill

template<>
struct serde::Serializable<
    app::handlers::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::revenue_recovery_data_backfill::
                                       RevenueRecoveryDataBackfillResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "revenue_recovery_data_backfill_result";
    }
};

template<>
struct serde::Serializable<app::handlers::revenue_recovery_data_backfill::RetryStatsMigrationResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "retry_stats_migration_result";
    }
};

template<>
struct serde::Serializable<app::handlers::revenue_recovery_data_backfill::UpdateRedisDataResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "update_redis_data_result";
    }
};

template<>
struct serde::Serializable<app::handlers::revenue_recovery_data_backfill::UnlockStatusResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::revenue_recovery_data_backfill::UnlockStatusResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "unlock_status_result";
    }
};
