export module app_user:customers_runtime_migrate_global_id;

import std;
import core_error;
import core_serde;
import app_context;
import app_store;
import app_model;

import :customers_input;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersMigrateRuntime
{
public:
    explicit CustomersMigrateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    migrate_global_id(const CustomersMigrateRouteInput& input) const
    {
        auto repo = m_context.repository();
        std::vector<::app::model::customers::CustomerGlobalIdMigrationRowResult> results{};
        results.reserve(input.rows.size());
        for (const auto& row: input.rows) {
            if (!std::holds_alternative<MigrationRow>(row)) {
                results.push_back(m_support.build_migration_result(
                    std::get<::app::model::customers::CustomerGlobalIdMigrationRowResult>(row)
                ));
            } else {
                results.push_back(m_support.process_fetched_row(repo, std::get<MigrationRow>(row)));
            }
        }
        const std::size_t total = results.size();
        std::size_t updated{0}, skipped{0};
        for (const auto& result: results) {
            const auto& status = result.get_status();
            if (status == "UpdatedNullId" || status == "UpdatedNonGlobalId") {
                ++updated;
            }
            if (status == "AlreadyGlobalId" || status == "SkippedNonV1") {
                ++skipped;
            }
        }
        serde::Value::Array data{};
        data.reserve(results.size());
        for (const auto& result: results) {
            data.push_back(m_support.migration_row_document(result));
        }
        serde::Value::Object response{};
        response["total_rows"] = serde::Value{static_cast<std::int64_t>(total)};
        response["updated_count"] = serde::Value{static_cast<std::int64_t>(updated)};
        response["skipped_count"] = serde::Value{static_cast<std::int64_t>(skipped)};
        response["failed_count"] =
            serde::Value{static_cast<std::int64_t>(total - updated - skipped)};
        response["results"] = serde::Value{std::move(data)};
        m_context.metrics().record_flow("customers.migrate_global_id");
        m_context.metrics().counter("customers_migrate_global_id").increment();
        return serde::Value{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
