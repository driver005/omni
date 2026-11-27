export module app_routing:blocklist_runtime_list_batch_blocklist_jobs;

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

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime_common;

export namespace app::handlers::blocklist {

class ListBatchBlocklistJobsRuntime
{
public:
    explicit ListBatchBlocklistJobsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // batch.rs:657-697 list_batch_blocklist_jobs.
    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::ListBatchBlocklistJobsResponse>
    list_batch_blocklist_jobs(const ::core::behavior::CallContext& ctx) const
    {
        const std::string job_type = m_support.query_value(ctx, "job_type").value_or("");
        const std::uint32_t limit = m_support.query_number<std::uint32_t>(ctx, "limit", 10);
        const std::uint32_t offset = m_support.query_number<std::uint32_t>(ctx, "offset", 0);
        std::vector<JobRecord> selected;
        for (auto& record: m_support.list_jobs("blocklist.job:")) {
            const bool merchant_ok = ctx.merchant_id.empty() || record.get_merchant_id().empty() ||
                                     record.get_merchant_id() == ctx.merchant_id;
            const bool profile_ok = ctx.profile_id.empty() || record.get_profile_id().empty() ||
                                    record.get_profile_id() == ctx.profile_id;
            if (!merchant_ok || !profile_ok) {
                continue;
            }
            if (!job_type.empty() && record.get_name() != job_type) {
                continue;
            }
            selected.push_back(std::move(record));
        }
        const std::size_t total = selected.size();
        auto page = m_support.paginate(std::move(selected), offset, limit);

        std::vector<::app::model::blocklist::BatchBlocklistJobStatusResponse> data;
        data.reserve(page.size());
        for (const auto& record: page) {
            auto job = m_support.job_status_of(record);
            // api_models:228: listing never mints a download_url per row.
            job.set_download_url(std::nullopt);
            job.set_download_url_expires_at(std::nullopt);
            data.push_back(std::move(job));
        }
        ::app::model::blocklist::ListBatchBlocklistJobsResponse response;
        response.set_count(data.size());
        response.set_total_count(total);
        response.set_data(std::move(data));
        m_support.emit("blocklist.jobs.list", response);
        m_support.track("list_batch_blocklist_jobs");
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntimeSupport m_support;
};

} // namespace app::handlers::blocklist
