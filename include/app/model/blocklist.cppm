module;
export module app_model:blocklist;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
import app_connector;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::blocklist {


class BatchBlocklistJobTypeValue : public core::behavior::Serializer<BatchBlocklistJobTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class BlocklistDataKindValue : public core::behavior::Serializer<BlocklistDataKindValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class BlocklistLookupDataValue : public core::behavior::Serializer<BlocklistLookupDataValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class BlocklistRequest
{
    CardBin,
    Fingerprint,
    ExtendedCardBin,
    GenericCardBin,
};

class BlocklistTransactionTypeValue :
    public core::behavior::Serializer<BlocklistTransactionTypeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class Card : public core::behavior::Serializer<Card>
{
public:
    void set_card_number(std::string value)
    {
        card_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_number() const noexcept
    {
        return card_number;
    }

    std::string card_number{};
};

class GenerateFingerprintRequest : public core::behavior::Serializer<GenerateFingerprintRequest>
{
public:
    void set_data(std::string value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::string& get_data() const noexcept
    {
        return data;
    }

    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    std::string data{};
    std::string key{};
};

class GenerateFingerprintResponsePayload :
    public core::behavior::Serializer<GenerateFingerprintResponsePayload>
{
public:
    void set_fingerprint_id(std::string value)
    {
        fingerprint_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_fingerprint_id() const noexcept
    {
        return fingerprint_id;
    }

    std::string fingerprint_id{};
};

class PageOffsetValue : public core::behavior::Serializer<PageOffsetValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PageSizeValue : public core::behavior::Serializer<PageSizeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class PrimitiveDateTimeValue : public core::behavior::Serializer<PrimitiveDateTimeValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ToggleBlocklistQuery : public core::behavior::Serializer<ToggleBlocklistQuery>
{
public:
    void set_status(bool value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const bool& get_status() const noexcept
    {
        return status;
    }

    bool status{};
};

class ToggleBlocklistResponse : public core::behavior::Serializer<ToggleBlocklistResponse>
{
public:
    void set_blocklist_guard_status(std::string value)
    {
        blocklist_guard_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_blocklist_guard_status() const noexcept
    {
        return blocklist_guard_status;
    }

    std::string blocklist_guard_status{};
};

class BatchBlocklistUploadQuery : public core::behavior::Serializer<BatchBlocklistUploadQuery>
{
public:
    void set_transaction_type(std::optional<BlocklistTransactionTypeValue> value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BlocklistTransactionTypeValue>&
    get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    std::optional<BlocklistTransactionTypeValue> transaction_type{};
};

class BatchBlocklistUploadResponse : public core::behavior::Serializer<BatchBlocklistUploadResponse>
{
public:
    void set_job_id(std::string value)
    {
        job_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_job_id() const noexcept
    {
        return job_id;
    }

    void set_total_rows(std::uint32_t value)
    {
        total_rows = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_total_rows() const noexcept
    {
        return total_rows;
    }

    void set_status(BatchBlocklistJobStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const BatchBlocklistJobStatusValue& get_status() const noexcept
    {
        return status;
    }

    std::string job_id{};
    std::uint32_t total_rows{};
    BatchBlocklistJobStatusValue status{};
};

class BlocklistCountQuery : public core::behavior::Serializer<BlocklistCountQuery>
{
public:
    void set_data_kind(BlocklistDataKindValue value)
    {
        data_kind = std::move(value);
    }

    [[nodiscard]] const BlocklistDataKindValue& get_data_kind() const noexcept
    {
        return data_kind;
    }

    void set_transaction_type(std::optional<BlocklistTransactionTypeValue> value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BlocklistTransactionTypeValue>&
    get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    BlocklistDataKindValue data_kind{};
    std::optional<BlocklistTransactionTypeValue> transaction_type{};
};

class BlocklistCountResponse : public core::behavior::Serializer<BlocklistCountResponse>
{
public:
    void set_data_kind(BlocklistDataKindValue value)
    {
        data_kind = std::move(value);
    }

    [[nodiscard]] const BlocklistDataKindValue& get_data_kind() const noexcept
    {
        return data_kind;
    }

    void set_total_count(std::size_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_total_count() const noexcept
    {
        return total_count;
    }

    // Upstream api_models:126 uses BTreeMap<usize, usize>; keys are ordered numerically.
    void set_counts_by_length(std::optional<std::map<std::string, std::size_t>> value)
    {
        counts_by_length = std::move(value);
    }

    [[nodiscard]] const std::optional<std::map<std::string, std::size_t>>&
    get_counts_by_length() const noexcept
    {
        return counts_by_length;
    }

    BlocklistDataKindValue data_kind{};
    std::size_t total_count{};
    std::optional<std::map<std::string, std::size_t>> counts_by_length{};
};

class BlocklistEntryRequest : public core::behavior::Serializer<BlocklistEntryRequest>
{
public:
    void set_entry(BlocklistRequest value)
    {
        entry = std::move(value);
    }

    [[nodiscard]] const BlocklistRequest& get_entry() const noexcept
    {
        return entry;
    }

    void set_transaction_type(BlocklistTransactionTypeValue value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const BlocklistTransactionTypeValue& get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    BlocklistRequest entry{};
    BlocklistTransactionTypeValue transaction_type{};
};

class BlocklistExportQuery : public core::behavior::Serializer<BlocklistExportQuery>
{
public:
    void set_transaction_type(std::optional<BlocklistTransactionTypeValue> value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BlocklistTransactionTypeValue>&
    get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    std::optional<BlocklistTransactionTypeValue> transaction_type{};
};

class BlocklistExportResponse : public core::behavior::Serializer<BlocklistExportResponse>
{
public:
    void set_export_id(std::string value)
    {
        export_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_export_id() const noexcept
    {
        return export_id;
    }

    void set_status(BatchBlocklistJobStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const BatchBlocklistJobStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_file_name(std::string value)
    {
        file_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_file_name() const noexcept
    {
        return file_name;
    }

    std::string export_id{};
    BatchBlocklistJobStatusValue status{};
    std::string file_name{};
};

class BlocklistLookupQuery : public core::behavior::Serializer<BlocklistLookupQuery>
{
public:
    void set_data(BlocklistLookupDataValue value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const BlocklistLookupDataValue& get_data() const noexcept
    {
        return data;
    }

    void set_transaction_type(std::optional<BlocklistTransactionTypeValue> value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BlocklistTransactionTypeValue>&
    get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    BlocklistLookupDataValue data{};
    std::optional<BlocklistTransactionTypeValue> transaction_type{};
};

class BlocklistLookupResponse : public core::behavior::Serializer<BlocklistLookupResponse>
{
public:
    void set_data(std::string value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::string& get_data() const noexcept
    {
        return data;
    }

    void set_blocked(bool value)
    {
        blocked = std::move(value);
    }

    [[nodiscard]] const bool& get_blocked() const noexcept
    {
        return blocked;
    }

    void set_transaction_types(std::vector<BlocklistTransactionTypeValue> value)
    {
        transaction_types = std::move(value);
    }

    [[nodiscard]] const std::vector<BlocklistTransactionTypeValue>&
    get_transaction_types() const noexcept
    {
        return transaction_types;
    }

    std::string data{};
    bool blocked{};
    std::vector<BlocklistTransactionTypeValue> transaction_types{};
};

class BlocklistResponse : public core::behavior::Serializer<BlocklistResponse>
{
public:
    void set_fingerprint_id(std::string value)
    {
        fingerprint_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_fingerprint_id() const noexcept
    {
        return fingerprint_id;
    }

    void set_data_kind(BlocklistDataKindValue value)
    {
        data_kind = std::move(value);
    }

    [[nodiscard]] const BlocklistDataKindValue& get_data_kind() const noexcept
    {
        return data_kind;
    }

    void set_transaction_type(BlocklistTransactionTypeValue value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const BlocklistTransactionTypeValue& get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    std::string fingerprint_id{};
    BlocklistDataKindValue data_kind{};
    BlocklistTransactionTypeValue transaction_type{};
    PrimitiveDateTimeValue created_at{};
    std::optional<ProfileIdValue> profile_id{};
};

class BlocklistRowError : public core::behavior::Serializer<BlocklistRowError>
{
public:
    void set_row_index(std::size_t value)
    {
        row_index = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_row_index() const noexcept
    {
        return row_index;
    }

    void set_data_kind(BlocklistDataKindValue value)
    {
        data_kind = std::move(value);
    }

    [[nodiscard]] const BlocklistDataKindValue& get_data_kind() const noexcept
    {
        return data_kind;
    }

    void set_data(std::string value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::string& get_data() const noexcept
    {
        return data;
    }

    void set_reason(std::string value)
    {
        reason = std::move(value);
    }

    [[nodiscard]] const std::string& get_reason() const noexcept
    {
        return reason;
    }

    std::size_t row_index{};
    BlocklistDataKindValue data_kind{};
    std::string data{};
    std::string reason{};
};

class CloneBlocklistEntriesRequest : public core::behavior::Serializer<CloneBlocklistEntriesRequest>
{
public:
    void set_target_profile_ids(std::vector<ProfileIdValue> value)
    {
        target_profile_ids = std::move(value);
    }

    [[nodiscard]] const std::vector<ProfileIdValue>& get_target_profile_ids() const noexcept
    {
        return target_profile_ids;
    }

    void set_transaction_type(std::optional<BlocklistTransactionTypeValue> value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BlocklistTransactionTypeValue>&
    get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    std::vector<ProfileIdValue> target_profile_ids{};
    std::optional<BlocklistTransactionTypeValue> transaction_type{};
};

class CloneBlocklistEntriesResponse :
    public core::behavior::Serializer<CloneBlocklistEntriesResponse>
{
public:
    void set_source_profile_id(ProfileIdValue value)
    {
        source_profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_source_profile_id() const noexcept
    {
        return source_profile_id;
    }

    void set_job_id(std::string value)
    {
        job_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_job_id() const noexcept
    {
        return job_id;
    }

    void set_status(BatchBlocklistJobStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const BatchBlocklistJobStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_target_profile_ids(std::vector<ProfileIdValue> value)
    {
        target_profile_ids = std::move(value);
    }

    [[nodiscard]] const std::vector<ProfileIdValue>& get_target_profile_ids() const noexcept
    {
        return target_profile_ids;
    }

    ProfileIdValue source_profile_id{};
    std::string job_id{};
    BatchBlocklistJobStatusValue status{};
    std::vector<ProfileIdValue> target_profile_ids{};
};

class ListBatchBlocklistJobsQuery : public core::behavior::Serializer<ListBatchBlocklistJobsQuery>
{
public:
    void set_limit(PageSizeValue value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const PageSizeValue& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(PageOffsetValue value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const PageOffsetValue& get_offset() const noexcept
    {
        return offset;
    }

    void set_job_type(std::optional<BatchBlocklistJobTypeValue> value)
    {
        job_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BatchBlocklistJobTypeValue>& get_job_type() const noexcept
    {
        return job_type;
    }

    PageSizeValue limit{};
    PageOffsetValue offset{};
    std::optional<BatchBlocklistJobTypeValue> job_type{};
};

class ListBlocklistQuery : public core::behavior::Serializer<ListBlocklistQuery>
{
public:
    void set_data_kind(BlocklistDataKindValue value)
    {
        data_kind = std::move(value);
    }

    [[nodiscard]] const BlocklistDataKindValue& get_data_kind() const noexcept
    {
        return data_kind;
    }

    void set_transaction_type(BlocklistTransactionTypeValue value)
    {
        transaction_type = std::move(value);
    }

    [[nodiscard]] const BlocklistTransactionTypeValue& get_transaction_type() const noexcept
    {
        return transaction_type;
    }

    void set_limit(std::uint16_t value)
    {
        limit = std::move(value);
    }

    [[nodiscard]] const std::uint16_t& get_limit() const noexcept
    {
        return limit;
    }

    void set_offset(std::uint16_t value)
    {
        offset = std::move(value);
    }

    [[nodiscard]] const std::uint16_t& get_offset() const noexcept
    {
        return offset;
    }

    BlocklistDataKindValue data_kind{};
    BlocklistTransactionTypeValue transaction_type{};
    std::uint16_t limit{};
    std::uint16_t offset{};
};

class ListBlocklistResponse : public core::behavior::Serializer<ListBlocklistResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::size_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<BlocklistResponse> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<BlocklistResponse>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::size_t total_count{};
    std::vector<BlocklistResponse> data{};
};

class BatchBlocklistJobStatusResponse :
    public core::behavior::Serializer<BatchBlocklistJobStatusResponse>
{
public:
    void set_job_id(std::string value)
    {
        job_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_job_id() const noexcept
    {
        return job_id;
    }

    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_profile_id(std::optional<ProfileIdValue> value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileIdValue>& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_job_type(BatchBlocklistJobTypeValue value)
    {
        job_type = std::move(value);
    }

    [[nodiscard]] const BatchBlocklistJobTypeValue& get_job_type() const noexcept
    {
        return job_type;
    }

    void set_file_name(std::optional<std::string> value)
    {
        file_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_file_name() const noexcept
    {
        return file_name;
    }

    void set_status(BatchBlocklistJobStatusValue value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const BatchBlocklistJobStatusValue& get_status() const noexcept
    {
        return status;
    }

    void set_total_rows(std::uint32_t value)
    {
        total_rows = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_total_rows() const noexcept
    {
        return total_rows;
    }

    void set_succeeded_rows(std::uint32_t value)
    {
        succeeded_rows = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_succeeded_rows() const noexcept
    {
        return succeeded_rows;
    }

    void set_failed_rows(std::uint32_t value)
    {
        failed_rows = std::move(value);
    }

    [[nodiscard]] const std::uint32_t& get_failed_rows() const noexcept
    {
        return failed_rows;
    }

    void set_metadata(std::optional<ProfileCloneJobMetadata> value)
    {
        metadata = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileCloneJobMetadata>& get_metadata() const noexcept
    {
        return metadata;
    }

    void set_created_at(PrimitiveDateTimeValue value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_updated_at(PrimitiveDateTimeValue value)
    {
        updated_at = std::move(value);
    }

    [[nodiscard]] const PrimitiveDateTimeValue& get_updated_at() const noexcept
    {
        return updated_at;
    }

    void set_expires_at(std::optional<PrimitiveDateTimeValue> value)
    {
        expires_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_expires_at() const noexcept
    {
        return expires_at;
    }

    void set_downloadable(bool value)
    {
        downloadable = std::move(value);
    }

    [[nodiscard]] const bool& get_downloadable() const noexcept
    {
        return downloadable;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_download_url(std::optional<std::string> value)
    {
        download_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_download_url() const noexcept
    {
        return download_url;
    }

    void set_download_url_expires_at(std::optional<PrimitiveDateTimeValue> value)
    {
        download_url_expires_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_download_url_expires_at() const noexcept
    {
        return download_url_expires_at;
    }

    std::string job_id{};
    std::string merchant_id{};
    std::optional<ProfileIdValue> profile_id{};
    BatchBlocklistJobTypeValue job_type{};
    std::optional<std::string> file_name{};
    BatchBlocklistJobStatusValue status{};
    std::uint32_t total_rows{};
    std::uint32_t succeeded_rows{};
    std::uint32_t failed_rows{};
    std::optional<ProfileCloneJobMetadata> metadata{};
    PrimitiveDateTimeValue created_at{};
    PrimitiveDateTimeValue updated_at{};
    std::optional<PrimitiveDateTimeValue> expires_at{};
    bool downloadable{};
    std::optional<std::string> error_message{};
    std::optional<std::string> download_url{};
    std::optional<PrimitiveDateTimeValue> download_url_expires_at{};
};

class ListBatchBlocklistJobsResponse :
    public core::behavior::Serializer<ListBatchBlocklistJobsResponse>
{
public:
    void set_count(std::size_t value)
    {
        count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_count() const noexcept
    {
        return count;
    }

    void set_total_count(std::size_t value)
    {
        total_count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_total_count() const noexcept
    {
        return total_count;
    }

    void set_data(std::vector<BatchBlocklistJobStatusResponse> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::vector<BatchBlocklistJobStatusResponse>& get_data() const noexcept
    {
        return data;
    }

    std::size_t count{};
    std::size_t total_count{};
    std::vector<BatchBlocklistJobStatusResponse> data{};
};
} // namespace app::model::blocklist

template<>
struct serde::Serializable<app::model::blocklist::GenerateFingerprintRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data",
                &app::model::blocklist::GenerateFingerprintRequest::get_data,
                &app::model::blocklist::GenerateFingerprintRequest::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "key",
                &app::model::blocklist::GenerateFingerprintRequest::get_key,
                &app::model::blocklist::GenerateFingerprintRequest::set_key,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "generate_fingerprint_request";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::Card>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "card_number",
            &app::model::blocklist::Card::get_card_number,
            &app::model::blocklist::Card::set_card_number,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "card";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistEntryRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "entry",
                &app::model::blocklist::BlocklistEntryRequest::get_entry,
                &app::model::blocklist::BlocklistEntryRequest::set_entry,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_type",
                &app::model::blocklist::BlocklistEntryRequest::get_transaction_type,
                &app::model::blocklist::BlocklistEntryRequest::set_transaction_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_entry_request";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "fingerprint_id",
                &app::model::blocklist::BlocklistResponse::get_fingerprint_id,
                &app::model::blocklist::BlocklistResponse::set_fingerprint_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data_kind",
                &app::model::blocklist::BlocklistResponse::get_data_kind,
                &app::model::blocklist::BlocklistResponse::set_data_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_type",
                &app::model::blocklist::BlocklistResponse::get_transaction_type,
                &app::model::blocklist::BlocklistResponse::set_transaction_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::blocklist::BlocklistResponse::get_created_at,
                &app::model::blocklist::BlocklistResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::blocklist::BlocklistResponse::get_profile_id,
                &app::model::blocklist::BlocklistResponse::set_profile_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::GenerateFingerprintResponsePayload>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "fingerprint_id",
            &app::model::blocklist::GenerateFingerprintResponsePayload::get_fingerprint_id,
            &app::model::blocklist::GenerateFingerprintResponsePayload::set_fingerprint_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "generate_fingerprint_response_payload";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::ToggleBlocklistResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "blocklist_guard_status",
            &app::model::blocklist::ToggleBlocklistResponse::get_blocklist_guard_status,
            &app::model::blocklist::ToggleBlocklistResponse::set_blocklist_guard_status,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "toggle_blocklist_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::ListBlocklistResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::blocklist::ListBlocklistResponse::get_count,
                &app::model::blocklist::ListBlocklistResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::blocklist::ListBlocklistResponse::get_total_count,
                &app::model::blocklist::ListBlocklistResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::blocklist::ListBlocklistResponse::get_data,
                &app::model::blocklist::ListBlocklistResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_blocklist_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::ListBlocklistQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data_kind",
                &app::model::blocklist::ListBlocklistQuery::get_data_kind,
                &app::model::blocklist::ListBlocklistQuery::set_data_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_type",
                &app::model::blocklist::ListBlocklistQuery::get_transaction_type,
                &app::model::blocklist::ListBlocklistQuery::set_transaction_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "limit",
                &app::model::blocklist::ListBlocklistQuery::get_limit,
                &app::model::blocklist::ListBlocklistQuery::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::blocklist::ListBlocklistQuery::get_offset,
                &app::model::blocklist::ListBlocklistQuery::set_offset,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_blocklist_query";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::ToggleBlocklistQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "status",
            &app::model::blocklist::ToggleBlocklistQuery::get_status,
            &app::model::blocklist::ToggleBlocklistQuery::set_status,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "toggle_blocklist_query";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistCountQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data_kind",
                &app::model::blocklist::BlocklistCountQuery::get_data_kind,
                &app::model::blocklist::BlocklistCountQuery::set_data_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_type",
                &app::model::blocklist::BlocklistCountQuery::get_transaction_type,
                &app::model::blocklist::BlocklistCountQuery::set_transaction_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_count_query";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistCountResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data_kind",
                &app::model::blocklist::BlocklistCountResponse::get_data_kind,
                &app::model::blocklist::BlocklistCountResponse::set_data_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::blocklist::BlocklistCountResponse::get_total_count,
                &app::model::blocklist::BlocklistCountResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "counts_by_length",
                &app::model::blocklist::BlocklistCountResponse::get_counts_by_length,
                &app::model::blocklist::BlocklistCountResponse::set_counts_by_length,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_count_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistLookupQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data",
                &app::model::blocklist::BlocklistLookupQuery::get_data,
                &app::model::blocklist::BlocklistLookupQuery::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_type",
                &app::model::blocklist::BlocklistLookupQuery::get_transaction_type,
                &app::model::blocklist::BlocklistLookupQuery::set_transaction_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_lookup_query";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistLookupResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data",
                &app::model::blocklist::BlocklistLookupResponse::get_data,
                &app::model::blocklist::BlocklistLookupResponse::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "blocked",
                &app::model::blocklist::BlocklistLookupResponse::get_blocked,
                &app::model::blocklist::BlocklistLookupResponse::set_blocked,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_types",
                &app::model::blocklist::BlocklistLookupResponse::get_transaction_types,
                &app::model::blocklist::BlocklistLookupResponse::set_transaction_types,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_lookup_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistRowError>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "row_index",
                &app::model::blocklist::BlocklistRowError::get_row_index,
                &app::model::blocklist::BlocklistRowError::set_row_index,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data_kind",
                &app::model::blocklist::BlocklistRowError::get_data_kind,
                &app::model::blocklist::BlocklistRowError::set_data_kind,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::blocklist::BlocklistRowError::get_data,
                &app::model::blocklist::BlocklistRowError::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reason",
                &app::model::blocklist::BlocklistRowError::get_reason,
                &app::model::blocklist::BlocklistRowError::set_reason,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_row_error";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BatchBlocklistUploadQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "transaction_type",
            &app::model::blocklist::BatchBlocklistUploadQuery::get_transaction_type,
            &app::model::blocklist::BatchBlocklistUploadQuery::set_transaction_type,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "batch_blocklist_upload_query";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BatchBlocklistUploadResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "job_id",
                &app::model::blocklist::BatchBlocklistUploadResponse::get_job_id,
                &app::model::blocklist::BatchBlocklistUploadResponse::set_job_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_rows",
                &app::model::blocklist::BatchBlocklistUploadResponse::get_total_rows,
                &app::model::blocklist::BatchBlocklistUploadResponse::set_total_rows,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::blocklist::BatchBlocklistUploadResponse::get_status,
                &app::model::blocklist::BatchBlocklistUploadResponse::set_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "batch_blocklist_upload_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BatchBlocklistJobStatusResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "job_id",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_job_id,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_job_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_merchant_id,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_profile_id,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "job_type",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_job_type,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_job_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_name",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_file_name,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_file_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_status,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_rows",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_total_rows,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_total_rows,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "succeeded_rows",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_succeeded_rows,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_succeeded_rows,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "failed_rows",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_failed_rows,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_failed_rows,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "metadata",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_metadata,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_metadata,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_created_at,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_updated_at,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_updated_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_at",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_expires_at,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_expires_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "downloadable",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_downloadable,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_downloadable,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_error_message,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "download_url",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::get_download_url,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::set_download_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "download_url_expires_at",
                &app::model::blocklist::BatchBlocklistJobStatusResponse::
                    get_download_url_expires_at,
                &app::model::blocklist::BatchBlocklistJobStatusResponse::
                    set_download_url_expires_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "batch_blocklist_job_status_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::ListBatchBlocklistJobsQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "limit",
                &app::model::blocklist::ListBatchBlocklistJobsQuery::get_limit,
                &app::model::blocklist::ListBatchBlocklistJobsQuery::set_limit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "offset",
                &app::model::blocklist::ListBatchBlocklistJobsQuery::get_offset,
                &app::model::blocklist::ListBatchBlocklistJobsQuery::set_offset,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "job_type",
                &app::model::blocklist::ListBatchBlocklistJobsQuery::get_job_type,
                &app::model::blocklist::ListBatchBlocklistJobsQuery::set_job_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_batch_blocklist_jobs_query";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::ListBatchBlocklistJobsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "count",
                &app::model::blocklist::ListBatchBlocklistJobsResponse::get_count,
                &app::model::blocklist::ListBatchBlocklistJobsResponse::set_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_count",
                &app::model::blocklist::ListBatchBlocklistJobsResponse::get_total_count,
                &app::model::blocklist::ListBatchBlocklistJobsResponse::set_total_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::blocklist::ListBatchBlocklistJobsResponse::get_data,
                &app::model::blocklist::ListBatchBlocklistJobsResponse::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "list_batch_blocklist_jobs_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistExportQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "transaction_type",
            &app::model::blocklist::BlocklistExportQuery::get_transaction_type,
            &app::model::blocklist::BlocklistExportQuery::set_transaction_type,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_export_query";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::BlocklistExportResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "export_id",
                &app::model::blocklist::BlocklistExportResponse::get_export_id,
                &app::model::blocklist::BlocklistExportResponse::set_export_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::blocklist::BlocklistExportResponse::get_status,
                &app::model::blocklist::BlocklistExportResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_name",
                &app::model::blocklist::BlocklistExportResponse::get_file_name,
                &app::model::blocklist::BlocklistExportResponse::set_file_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "blocklist_export_response";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::CloneBlocklistEntriesRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "target_profile_ids",
                &app::model::blocklist::CloneBlocklistEntriesRequest::get_target_profile_ids,
                &app::model::blocklist::CloneBlocklistEntriesRequest::set_target_profile_ids,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_type",
                &app::model::blocklist::CloneBlocklistEntriesRequest::get_transaction_type,
                &app::model::blocklist::CloneBlocklistEntriesRequest::set_transaction_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "clone_blocklist_entries_request";
    }
};

template<>
struct serde::Serializable<app::model::blocklist::CloneBlocklistEntriesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "source_profile_id",
                &app::model::blocklist::CloneBlocklistEntriesResponse::get_source_profile_id,
                &app::model::blocklist::CloneBlocklistEntriesResponse::set_source_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "job_id",
                &app::model::blocklist::CloneBlocklistEntriesResponse::get_job_id,
                &app::model::blocklist::CloneBlocklistEntriesResponse::set_job_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::model::blocklist::CloneBlocklistEntriesResponse::get_status,
                &app::model::blocklist::CloneBlocklistEntriesResponse::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "target_profile_ids",
                &app::model::blocklist::CloneBlocklistEntriesResponse::get_target_profile_ids,
                &app::model::blocklist::CloneBlocklistEntriesResponse::set_target_profile_ids,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "clone_blocklist_entries_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::model::blocklist::tests {
using namespace boost::ut;
suite<"app_model_blocklist"> app_model_blocklist_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::model::blocklist::GenerateFingerprintRequest>::fields())> > 0
        );
    };
};
} // namespace app::model::blocklist::tests
#endif
