export module app_routing:blocklist_result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::blocklist {

// Every old blocklist response was `serde::Ser::encode_json(response)` of an
// ::app::model::blocklist response DTO. Each Result inherits that exact DTO and
// reuses its FieldDesc tuple, so the JSON wire shape stays byte-identical.

class AddEntryResult : public ::app::model::blocklist::BlocklistResponse
{
public:
    AddEntryResult() = default;

    explicit AddEntryResult(::app::model::blocklist::BlocklistResponse value) noexcept :
        BlocklistResponse{std::move(value)}
    {
    }
};

class RemoveEntryResult : public ::app::model::blocklist::BlocklistResponse
{
public:
    RemoveEntryResult() = default;

    explicit RemoveEntryResult(::app::model::blocklist::BlocklistResponse value) noexcept :
        BlocklistResponse{std::move(value)}
    {
    }
};

class ListBlockedPaymentMethodsResult : public ::app::model::blocklist::ListBlocklistResponse
{
public:
    ListBlockedPaymentMethodsResult() = default;

    explicit ListBlockedPaymentMethodsResult(
        ::app::model::blocklist::ListBlocklistResponse value
    ) noexcept :
        ListBlocklistResponse{std::move(value)}
    {
    }
};

class GetBlocklistCountResult : public ::app::model::blocklist::BlocklistCountResponse
{
public:
    GetBlocklistCountResult() = default;

    explicit GetBlocklistCountResult(::app::model::blocklist::BlocklistCountResponse value) noexcept
        :
        BlocklistCountResponse{std::move(value)}
    {
    }
};

class LookupBlocklistEntryResult : public ::app::model::blocklist::BlocklistLookupResponse
{
public:
    LookupBlocklistEntryResult() = default;

    explicit LookupBlocklistEntryResult(
        ::app::model::blocklist::BlocklistLookupResponse value
    ) noexcept :
        BlocklistLookupResponse{std::move(value)}
    {
    }
};

class ToggleBlocklistGuardResult : public ::app::model::blocklist::ToggleBlocklistResponse
{
public:
    ToggleBlocklistGuardResult() = default;

    explicit ToggleBlocklistGuardResult(
        ::app::model::blocklist::ToggleBlocklistResponse value
    ) noexcept :
        ToggleBlocklistResponse{std::move(value)}
    {
    }
};

class UploadBatchBlocklistResult : public ::app::model::blocklist::BatchBlocklistUploadResponse
{
public:
    UploadBatchBlocklistResult() = default;

    explicit UploadBatchBlocklistResult(
        ::app::model::blocklist::BatchBlocklistUploadResponse value
    ) noexcept :
        BatchBlocklistUploadResponse{std::move(value)}
    {
    }
};

class GetBatchBlocklistJobStatusResult :
    public ::app::model::blocklist::BatchBlocklistJobStatusResponse
{
public:
    GetBatchBlocklistJobStatusResult() = default;

    explicit GetBatchBlocklistJobStatusResult(
        ::app::model::blocklist::BatchBlocklistJobStatusResponse value
    ) noexcept :
        BatchBlocklistJobStatusResponse{std::move(value)}
    {
    }
};

class ListBatchBlocklistJobsResult : public ::app::model::blocklist::ListBatchBlocklistJobsResponse
{
public:
    ListBatchBlocklistJobsResult() = default;

    explicit ListBatchBlocklistJobsResult(
        ::app::model::blocklist::ListBatchBlocklistJobsResponse value
    ) noexcept :
        ListBatchBlocklistJobsResponse{std::move(value)}
    {
    }
};

class CreateBlocklistExportResult : public ::app::model::blocklist::BlocklistExportResponse
{
public:
    CreateBlocklistExportResult() = default;

    explicit CreateBlocklistExportResult(
        ::app::model::blocklist::BlocklistExportResponse value
    ) noexcept :
        BlocklistExportResponse{std::move(value)}
    {
    }
};

class CloneBlocklistEntriesResult : public ::app::model::blocklist::CloneBlocklistEntriesResponse
{
public:
    CloneBlocklistEntriesResult() = default;

    explicit CloneBlocklistEntriesResult(
        ::app::model::blocklist::CloneBlocklistEntriesResponse value
    ) noexcept :
        CloneBlocklistEntriesResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::blocklist

template<>
struct serde::Serializable<::app::handlers::blocklist::AddEntryResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::blocklist::BlocklistResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_add_entry_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::RemoveEntryResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::blocklist::BlocklistResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_remove_entry_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::ListBlockedPaymentMethodsResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::blocklist::ListBlocklistResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_list_blocked_payment_methods_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::GetBlocklistCountResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::blocklist::BlocklistCountResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_get_blocklist_count_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::LookupBlocklistEntryResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::blocklist::BlocklistLookupResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_lookup_blocklist_entry_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::ToggleBlocklistGuardResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::blocklist::ToggleBlocklistResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_toggle_blocklist_guard_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::UploadBatchBlocklistResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::blocklist::BatchBlocklistUploadResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_upload_batch_blocklist_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::GetBatchBlocklistJobStatusResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::model::blocklist::BatchBlocklistJobStatusResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_get_batch_blocklist_job_status_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::ListBatchBlocklistJobsResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::model::blocklist::ListBatchBlocklistJobsResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_list_batch_blocklist_jobs_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::CreateBlocklistExportResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::model::blocklist::BlocklistExportResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_create_blocklist_export_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::blocklist::CloneBlocklistEntriesResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            ::app::model::blocklist::CloneBlocklistEntriesResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_clone_blocklist_entries_result";
    }
};
