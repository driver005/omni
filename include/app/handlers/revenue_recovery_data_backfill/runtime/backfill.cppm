export module app_handlers_revenue_recovery_data_backfill:runtime_backfill;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_crypto;
import app_locking;
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::revenue_recovery_data_backfill {

class BackfillRuntime
{
public:
    explicit BackfillRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:19-73 ->
     * core/revenue_recovery_data_backfill.rs:23-64.
     */
    [[nodiscard]] std::expected<RevenueRecoveryDataBackfillResult, core::error::AppError>
    backfill(const RevenueRecoveryDataBackfillInput& input) const
    {
        std::size_t processed_records{0};
        std::size_t failed_records{0};
        for (const auto& record: input.parsed.get_records()) {
            // core:32-49: process_payment_method_record; Ok -> processed, Err -> failed.
            if (m_support.process_payment_method_record(record, input.cutoff_datetime)) {
                ++processed_records;
            } else {
                ++failed_records;
            }
        }
        // api_models:69-73: rows rejected by the CSV parser are already failed records.
        failed_records += input.parsed.get_failed_records().size();

        m_support.record_flow(
            "RecoveryDataBackfill",
            "revenue_recovery_data_backfill",
            processed_records,
            failed_records
        );

        using Response =
            ::app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse;
        Response response;
        response.set_processed_records(processed_records);
        response.set_failed_records(failed_records);
        return RevenueRecoveryDataBackfillResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    RevenueRecoveryRuntimeSupport m_support;
};

} // namespace app::handlers::revenue_recovery_data_backfill
