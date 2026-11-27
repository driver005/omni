export module app_routing:blocklist_runtime_create_blocklist_export;

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
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import core_behavior;

import :blocklist_result;
import :blocklist_runtime_common;

export namespace app::handlers::blocklist {

class CreateBlocklistExportRuntime
{
public:
    explicit CreateBlocklistExportRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // export.rs:73-162 initiate_blocklist_export.
    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BlocklistExportResponse>
    create_blocklist_export(const ::core::behavior::CallContext& ctx) const
    {
        const std::string transaction_type =
            m_support.query_value(ctx, "transaction_type").value_or("");
        const auto export_id = m_context.ids().payment_id("blkexp");
        const auto stamp = m_support.now_iso8601();
        // export.rs:32-42: blocklist_export_YYYYMMDD_HHMMSS.csv.
        const auto file_name = "blocklist_export_" + stamp.substr(0, 4) + stamp.substr(5, 2) +
                               stamp.substr(8, 2) + "_" + stamp.substr(11, 2) +
                               stamp.substr(14, 2) + stamp.substr(17, 2) + ".csv";
        const auto csv = m_support.export_csv(ctx.merchant_id, ctx.profile_id, transaction_type);

        ::app::store::files::FileRecord file;
        file.set_file_id(export_id);
        file.set_merchant_id(ctx.merchant_id);
        file.set_profile_id(ctx.profile_id);
        file.set_file_name(file_name);
        file.set_file_type("text/csv");
        file.set_file_size(static_cast<std::int64_t>(csv.size()));
        file.set_purpose("blocklist_export");
        file.set_provider_file_id(file_name);
        file.set_upload_provider("local");
        file.set_status("available");
        file.set_created_at(stamp);
        m_context.repository().save_file(std::move(file));

        m_support.upsert_job(
            export_id,
            ctx.merchant_id,
            ctx.profile_id,
            "export",
            "initiated",
            0,
            transaction_type,
            file_name,
            {},
            false,
            std::nullopt,
            0,
            0,
            {},
            {},
            {}
        );
        // Worker completion substitute: export.rs:164-174 downloadable/expires_at.
        const auto rows =
            std::count(csv.begin(), csv.end(), '\n') > 0
                ? static_cast<std::uint32_t>(std::count(csv.begin(), csv.end(), '\n') - 1)
                : 0U;
        m_support.upsert_job(
            export_id,
            ctx.merchant_id,
            ctx.profile_id,
            "export",
            "completed",
            rows,
            transaction_type,
            file_name,
            {},
            true,
            std::nullopt,
            0,
            0,
            {},
            m_support.now_iso8601(86'400),
            m_support.now_iso8601(900)
        );
        m_support
            .schedule_job("BLOCKLIST_EXPORT", export_id, ctx.merchant_id, ctx.profile_id, rows);

        ::app::model::blocklist::BlocklistExportResponse response;
        response.set_export_id(export_id);
        response.set_status(
            m_support.wrap<::app::model::blocklist::BatchBlocklistJobStatusValue>("initiated")
        );
        response.set_file_name(file_name);
        m_support.emit("blocklist.export.created", response);
        m_support.track("create_blocklist_export");
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
