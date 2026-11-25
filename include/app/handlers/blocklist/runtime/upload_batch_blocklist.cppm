export module app_handlers_blocklist:runtime_upload_batch_blocklist;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::blocklist {

class UploadBatchBlocklistRuntime
{
public:
    explicit UploadBatchBlocklistRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // batch.rs:348-510 initiate_batch_blocklist_upload.
    [[nodiscard]] std::expected<UploadBatchBlocklistResult, core::error::AppError>
    upload_batch_blocklist(const UploadBatchBlocklistInput& input) const
    {
        const auto job_id = m_context.ids().payment_id("blkbatch");
        const auto stamp = m_support.now_iso8601();
        // in-process substitute for batch.rs:512-551 process_chunk (the worker is not present).
        std::uint32_t succeeded{0};
        for (const auto& row: input.rows) {
            const auto key =
                m_support
                    .identity_key(input.merchant, input.profile, row.data, row.transaction_type);
            if (m_support.find_entry(key)) {
                ++succeeded;
                continue;
            }
            m_support.save_entry(m_support.make_entry(
                row.data,
                input.merchant,
                input.profile,
                row.kind,
                row.transaction_type,
                stamp
            ));
            ++succeeded;
        }

        const auto total_rows = static_cast<std::uint32_t>(input.rows.size());
        m_support.upsert_job(
            job_id,
            input.merchant,
            input.profile,
            "upload",
            "initiated",
            total_rows,
            input.override_type,
            input.file_name,
            {},
            false,
            std::nullopt,
            succeeded,
            0,
            {}
        );
        // Worker completion: batch.rs:437-450 job row starts Initiated; the in-process
        // substitute updates it to Completed once the synchronous insert has run.
        m_support.upsert_job(
            job_id,
            input.merchant,
            input.profile,
            "upload",
            "completed",
            total_rows,
            input.override_type,
            input.file_name,
            {},
            false,
            std::nullopt,
            succeeded,
            0,
            {}
        );
        m_support.schedule_job(
            "BATCH_BLOCKLIST_UPLOAD",
            job_id,
            input.merchant,
            input.profile,
            input.rows.size()
        );

        app::model::blocklist::BatchBlocklistUploadResponse response;
        response.set_job_id(job_id);
        response.set_total_rows(total_rows);
        response.set_status(
            m_support.wrap<app::model::blocklist::BatchBlocklistJobStatusValue>("initiated")
        );
        m_support.emit("blocklist.batch.upload", response);
        m_support.track("upload_batch_blocklist");
        return UploadBatchBlocklistResult{response};
    }

private:
    app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
