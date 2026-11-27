export module app_payment:revenue_recovery_data_backfill_runtime;

import std;
import core_error;
import app_context;

import :revenue_recovery_data_backfill_input;
import :revenue_recovery_data_backfill_result;
import :revenue_recovery_data_backfill_runtime_common;
import :revenue_recovery_data_backfill_runtime_backfill;
import :revenue_recovery_data_backfill_runtime_retry_stats_migration;
import :revenue_recovery_data_backfill_runtime_update_redis_data;
import :revenue_recovery_data_backfill_runtime_unlock_status;

export namespace app::handlers::revenue_recovery_data_backfill {

class RevenueRecoveryDataBackfillRuntime
{
public:
    explicit RevenueRecoveryDataBackfillRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_backfill{context},
        m_retry_stats_migration{context},
        m_update_redis_data{context},
        m_unlock_status{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse>
        backfill(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_backfill.backfill(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse>
        retry_stats_migration(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retry_stats_migration.retry_stats_migration(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse>
        update_redis_data(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_update_redis_data.update_redis_data(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::revenue_recovery_data_backfill::UnlockStatusResponse>
        unlock_status(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_unlock_status.unlock_status(dto, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    BackfillRuntime m_backfill;
    RetryStatsMigrationRuntime m_retry_stats_migration;
    UpdateRedisDataRuntime m_update_redis_data;
    UnlockStatusRuntime m_unlock_status;
};

} // namespace app::handlers::revenue_recovery_data_backfill
