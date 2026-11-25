export module app_handlers_revenue_recovery_data_backfill:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_backfill;
import :runtime_retry_stats_migration;
import :runtime_update_redis_data;
import :runtime_unlock_status;

export namespace app::handlers::revenue_recovery_data_backfill {

class RevenueRecoveryDataBackfillRuntime
{
public:
    explicit RevenueRecoveryDataBackfillRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_backfill{context},
        m_retry_stats_migration{context},
        m_update_redis_data{context},
        m_unlock_status{context}
    {
    }

    [[nodiscard]] std::expected<RevenueRecoveryDataBackfillResult, core::error::AppError>
    backfill(const RevenueRecoveryDataBackfillInput& input) const
    {
        return m_backfill.backfill(input);
    }

    [[nodiscard]] std::expected<RetryStatsMigrationResult, core::error::AppError>
    retry_stats_migration(const RetryStatsMigrationInput& input) const
    {
        return m_retry_stats_migration.retry_stats_migration(input);
    }

    [[nodiscard]] std::expected<UpdateRedisDataResult, core::error::AppError>
    update_redis_data(const UpdateRedisDataInput& input) const
    {
        return m_update_redis_data.update_redis_data(input);
    }

    [[nodiscard]] std::expected<UnlockStatusResult, core::error::AppError>
    unlock_status(const UnlockStatusInput& input) const
    {
        return m_unlock_status.unlock_status(input);
    }

private:
    app::context::AppContext& m_context;
    BackfillRuntime m_backfill;
    RetryStatsMigrationRuntime m_retry_stats_migration;
    UpdateRedisDataRuntime m_update_redis_data;
    UnlockStatusRuntime m_unlock_status;
};

} // namespace app::handlers::revenue_recovery_data_backfill
