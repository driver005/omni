export module app_routing:blocklist_runtime;

import std;
import app_model;
import core_error;
import app_context;
import core_behavior;

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime_common;
import :blocklist_runtime_add_entry;
import :blocklist_runtime_remove_entry;
import :blocklist_runtime_list_blocked_payment_methods;
import :blocklist_runtime_get_blocklist_count;
import :blocklist_runtime_lookup_blocklist_entry;
import :blocklist_runtime_toggle_blocklist_guard;
import :blocklist_runtime_upload_batch_blocklist;
import :blocklist_runtime_get_batch_blocklist_job_status;
import :blocklist_runtime_list_batch_blocklist_jobs;
import :blocklist_runtime_create_blocklist_export;
import :blocklist_runtime_clone_blocklist_entries;

export namespace app::handlers::blocklist {

class BlocklistRuntime
{
public:
    explicit BlocklistRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_add_entry{context},
        m_remove_entry{context},
        m_list_blocked_payment_methods{context},
        m_get_blocklist_count{context},
        m_lookup_blocklist_entry{context},
        m_toggle_blocklist_guard{context},
        m_upload_batch_blocklist{context},
        m_get_batch_blocklist_job_status{context},
        m_list_batch_blocklist_jobs{context},
        m_create_blocklist_export{context},
        m_clone_blocklist_entries{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BlocklistResponse>
    add_entry(const EntryWire& entry, const ::core::behavior::CallContext& ctx) const
    {
        return m_add_entry.add_entry(entry, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BlocklistResponse>
    remove_entry(const EntryWire& entry, const ::core::behavior::CallContext& ctx) const
    {
        return m_remove_entry.remove_entry(entry, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::ListBlocklistResponse>
    list_blocked_payment_methods(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_blocked_payment_methods.list_blocked_payment_methods(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BlocklistCountResponse>
    get_blocklist_count(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_blocklist_count.get_blocklist_count(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BlocklistLookupResponse>
    lookup_blocklist_entry(const ::core::behavior::CallContext& ctx) const
    {
        return m_lookup_blocklist_entry.lookup_blocklist_entry(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::ToggleBlocklistResponse>
    toggle_blocklist_guard(const ::core::behavior::CallContext& ctx) const
    {
        return m_toggle_blocklist_guard.toggle_blocklist_guard(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BatchBlocklistUploadResponse>
    upload_batch_blocklist(
        const std::vector<CsvRow>& rows,
        std::string_view file_name,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_upload_batch_blocklist.upload_batch_blocklist(rows, file_name, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BatchBlocklistJobStatusResponse>
    get_batch_blocklist_job_status(
        const JobRecord& stored,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_batch_blocklist_job_status.get_batch_blocklist_job_status(stored, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::ListBatchBlocklistJobsResponse>
    list_batch_blocklist_jobs(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_batch_blocklist_jobs.list_batch_blocklist_jobs(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::BlocklistExportResponse>
    create_blocklist_export(const ::core::behavior::CallContext& ctx) const
    {
        return m_create_blocklist_export.create_blocklist_export(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::blocklist::CloneBlocklistEntriesResponse>
    clone_blocklist_entries(
        const ::app::model::blocklist::CloneBlocklistEntriesRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_clone_blocklist_entries.clone_blocklist_entries(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    AddEntryRuntime m_add_entry;
    RemoveEntryRuntime m_remove_entry;
    ListBlockedPaymentMethodsRuntime m_list_blocked_payment_methods;
    GetBlocklistCountRuntime m_get_blocklist_count;
    LookupBlocklistEntryRuntime m_lookup_blocklist_entry;
    ToggleBlocklistGuardRuntime m_toggle_blocklist_guard;
    UploadBatchBlocklistRuntime m_upload_batch_blocklist;
    GetBatchBlocklistJobStatusRuntime m_get_batch_blocklist_job_status;
    ListBatchBlocklistJobsRuntime m_list_batch_blocklist_jobs;
    CreateBlocklistExportRuntime m_create_blocklist_export;
    CloneBlocklistEntriesRuntime m_clone_blocklist_entries;
};

} // namespace app::handlers::blocklist
