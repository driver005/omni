export module app_routing:blocklist_runtime;

import std;
import app_model;
import core_error;
import app_context;

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

    [[nodiscard]] std::expected<::app::model::blocklist::BlocklistResponse, core::error::AppError>
    add_entry(const AddEntryInput& input) const
    {
        return m_add_entry.add_entry(input);
    }

    [[nodiscard]] std::expected<::app::model::blocklist::BlocklistResponse, core::error::AppError>
    remove_entry(const RemoveEntryInput& input) const
    {
        return m_remove_entry.remove_entry(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::ListBlocklistResponse, core::error::AppError>
        list_blocked_payment_methods(const ListBlockedPaymentMethodsInput& input) const
    {
        return m_list_blocked_payment_methods.list_blocked_payment_methods(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::BlocklistCountResponse, core::error::AppError>
        get_blocklist_count(const GetBlocklistCountInput& input) const
    {
        return m_get_blocklist_count.get_blocklist_count(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::BlocklistLookupResponse, core::error::AppError>
        lookup_blocklist_entry(const LookupBlocklistEntryInput& input) const
    {
        return m_lookup_blocklist_entry.lookup_blocklist_entry(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::ToggleBlocklistResponse, core::error::AppError>
        toggle_blocklist_guard(const ToggleBlocklistGuardInput& input) const
    {
        return m_toggle_blocklist_guard.toggle_blocklist_guard(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::BatchBlocklistUploadResponse, core::error::AppError>
        upload_batch_blocklist(const UploadBatchBlocklistInput& input) const
    {
        return m_upload_batch_blocklist.upload_batch_blocklist(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::BatchBlocklistJobStatusResponse, core::error::AppError>
        get_batch_blocklist_job_status(const GetBatchBlocklistJobStatusInput& input) const
    {
        return m_get_batch_blocklist_job_status.get_batch_blocklist_job_status(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::ListBatchBlocklistJobsResponse, core::error::AppError>
        list_batch_blocklist_jobs(const ListBatchBlocklistJobsInput& input) const
    {
        return m_list_batch_blocklist_jobs.list_batch_blocklist_jobs(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::BlocklistExportResponse, core::error::AppError>
        create_blocklist_export(const CreateBlocklistExportInput& input) const
    {
        return m_create_blocklist_export.create_blocklist_export(input);
    }

    [[nodiscard]] std::
        expected<::app::model::blocklist::CloneBlocklistEntriesResponse, core::error::AppError>
        clone_blocklist_entries(const CloneBlocklistEntriesInput& input) const
    {
        return m_clone_blocklist_entries.clone_blocklist_entries(input);
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
