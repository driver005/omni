export module app_payment:revenue_recovery_data_backfill_runtime_retry_stats_migration;
import core_behavior;

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

import :revenue_recovery_data_backfill_input;
import :revenue_recovery_data_backfill_result;
import :revenue_recovery_data_backfill_runtime_common;

export namespace app::handlers::revenue_recovery_data_backfill {

class RetryStatsMigrationRuntime
{
public:
    explicit RetryStatsMigrationRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:75-104 ->
     * core/revenue_recovery/retry_stats/migration.rs:21-130.
     */
    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse>
    retry_stats_migration(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RetryStatsMigrationData{dto, ctx};
        using Error = ::app::dto::revenue_recovery_data_backfill::CsvParsingError;
        const auto row_error = [](std::size_t row, std::string message)
        {
            Error error{};
            error.set_row_number(row);
            error.set_error(std::move(message));
            return error;
        };

        // migration.rs:25-60: phase 1 validates EVERY row before any write, then rejects
        // duplicate cluster keys. Nothing is persisted while validation errors exist.
        std::vector<std::pair<std::size_t, backfill_support::ValidatedRow>> validated_rows{};
        std::vector<Error> validation_errors{};
        std::unordered_set<std::string> seen_keys{};
        for (const auto& [row_number, record]: input.parsed.get_records()) {
            auto key = m_support.validate_cluster_key(record.get_cluster_key());
            if (!key) {
                validation_errors.push_back(
                    row_error(row_number, "invalid cluster_key '" + record.get_cluster_key() + "'")
                );
                continue;
            }
            std::string doc_error{};
            auto document = m_support.parse_stats_document(record.get_stats(), doc_error);
            if (!document) {
                validation_errors.push_back(
                    row_error(row_number, "stats is not a valid stats document: " + doc_error)
                );
                continue;
            }
            if (!seen_keys.insert(*key).second) {
                validation_errors.push_back(row_error(row_number, "duplicate cluster_key present"));
                continue;
            }
            validated_rows.emplace_back(
                row_number,
                backfill_support::ValidatedRow{*key, record.get_stats(), std::move(*document)}
            );
        }

        std::vector<Error> row_errors{};
        std::size_t processed_records{0};
        if (validation_errors.empty()) {
            // migration.rs:75-99: phase 2 persists sequentially; every write is a
            // whole-document replace, so re-running the file is safe.
            for (const auto& [row_number, row]: validated_rows) {
                const auto replaced = m_support.replace_retry_stats_document(row.key, row.stats);
                if (!replaced || !*replaced) {
                    row_errors.push_back(row_error(
                        row_number,
                        "per-key redis lock stayed contended; row skipped, retry the file"
                    ));
                }
            }
            processed_records = validated_rows.size() - row_errors.size();
        } else {
            row_errors = std::move(validation_errors);
        }

        m_support.record_flow(
            "RecoveryRetryStatsMigration",
            "revenue_recovery_retry_stats_migration",
            processed_records,
            row_errors.size()
        );

        using Response = ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse;
        Response response;
        response.set_processed_records(processed_records);
        response.set_failed_records(row_errors.size());
        response.set_row_errors(std::move(row_errors));
        return ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryRuntimeSupport m_support;
};

} // namespace app::handlers::revenue_recovery_data_backfill
