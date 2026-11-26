export module app_routing:blocklist_runtime_get_batch_blocklist_job_status;

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

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime_common;

export namespace app::handlers::blocklist {

class GetBatchBlocklistJobStatusRuntime
{
public:
    explicit GetBatchBlocklistJobStatusRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // batch.rs:553-590 get_batch_blocklist_job_status.
    [[nodiscard]] std::expected<GetBatchBlocklistJobStatusResult, core::error::AppError>
    get_batch_blocklist_job_status(const GetBatchBlocklistJobStatusInput& input) const
    {
        auto response = m_support.job_status_of(input.stored);
        // batch.rs:581-589: the only place a download link is minted.
        if (m_support.is_export_downloadable(response)) {
            const auto expires = m_support.now_iso8601(900);
            response.set_download_url(m_support.signed_download_url(
                response.get_file_name().value_or(std::string{}),
                expires
            ));
            response.set_download_url_expires_at(
                m_support.wrap<::app::model::blocklist::PrimitiveDateTimeValue>(expires)
            );
        }
        m_support.emit("blocklist.job.status", response);
        m_support.track("get_batch_blocklist_job_status");
        return GetBatchBlocklistJobStatusResult{response};
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
