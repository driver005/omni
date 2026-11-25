module;
export module app_store:ops_records;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::store::process_trackers {

/** @brief Persisted scheduled background task / process tracker. */
class ProcessTrackerRecord
{
public:
    ProcessTrackerRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_task_id(std::string value)
    {
        m_task_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_task_id() const noexcept
    {
        return m_task_id;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_payload(std::string value)
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_payload() const noexcept
    {
        return m_payload;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_result(std::string value)
    {
        m_result = std::move(value);
    }

    [[nodiscard]] const std::string& get_result() const noexcept
    {
        return m_result;
    }

    void set_run_at(std::string value)
    {
        m_run_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_run_at() const noexcept
    {
        return m_run_at;
    }

    void set_attempts(std::int64_t value)
    {
        m_attempts = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_attempts() const noexcept
    {
        return m_attempts;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    void set_updated_at(std::string value)
    {
        m_updated_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    std::int64_t m_id{};
    std::string m_task_id{};
    std::string m_name{};
    std::string m_payload{};
    std::string m_status{};
    std::string m_result{};
    std::string m_run_at{};
    std::int64_t m_attempts{};
    std::string m_created_at{};
    std::string m_updated_at{};
};

} // namespace app::store::process_trackers

template<>
struct serde::Serializable<app::store::process_trackers::ProcessTrackerRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::process_trackers::ProcessTrackerRecord::get_id,
                &app::store::process_trackers::ProcessTrackerRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "task_id",
                &app::store::process_trackers::ProcessTrackerRecord::get_task_id,
                &app::store::process_trackers::ProcessTrackerRecord::set_task_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &app::store::process_trackers::ProcessTrackerRecord::get_name,
                &app::store::process_trackers::ProcessTrackerRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payload",
                &app::store::process_trackers::ProcessTrackerRecord::get_payload,
                &app::store::process_trackers::ProcessTrackerRecord::set_payload,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::process_trackers::ProcessTrackerRecord::get_status,
                &app::store::process_trackers::ProcessTrackerRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "result",
                &app::store::process_trackers::ProcessTrackerRecord::get_result,
                &app::store::process_trackers::ProcessTrackerRecord::set_result,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "run_at",
                &app::store::process_trackers::ProcessTrackerRecord::get_run_at,
                &app::store::process_trackers::ProcessTrackerRecord::set_run_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "attempts",
                &app::store::process_trackers::ProcessTrackerRecord::get_attempts,
                &app::store::process_trackers::ProcessTrackerRecord::set_attempts,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::process_trackers::ProcessTrackerRecord::get_created_at,
                &app::store::process_trackers::ProcessTrackerRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &app::store::process_trackers::ProcessTrackerRecord::get_updated_at,
                &app::store::process_trackers::ProcessTrackerRecord::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "process_trackers";
    }
};

export namespace app::store::files {

/** @brief Persisted file metadata (bytes live in the upload provider). */
class FileRecord
{
public:
    FileRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_file_id(std::string value)
    {
        m_file_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_file_id() const noexcept
    {
        return m_file_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return m_profile_id;
    }

    void set_file_name(std::string value)
    {
        m_file_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_file_name() const noexcept
    {
        return m_file_name;
    }

    void set_file_type(std::string value)
    {
        m_file_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_file_type() const noexcept
    {
        return m_file_type;
    }

    void set_file_size(std::int64_t value)
    {
        m_file_size = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_file_size() const noexcept
    {
        return m_file_size;
    }

    void set_purpose(std::string value)
    {
        m_purpose = std::move(value);
    }

    [[nodiscard]] const std::string& get_purpose() const noexcept
    {
        return m_purpose;
    }

    void set_provider_file_id(std::string value)
    {
        m_provider_file_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_provider_file_id() const noexcept
    {
        return m_provider_file_id;
    }

    void set_upload_provider(std::string value)
    {
        m_upload_provider = std::move(value);
    }

    [[nodiscard]] const std::string& get_upload_provider() const noexcept
    {
        return m_upload_provider;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    std::int64_t m_id{};
    std::string m_file_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_file_name{};
    std::string m_file_type{};
    std::int64_t m_file_size{};
    std::string m_purpose{};
    std::string m_provider_file_id{};
    std::string m_upload_provider{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::files

template<>
struct serde::Serializable<app::store::files::FileRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::files::FileRecord::get_id,
                &app::store::files::FileRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "file_id",
                &app::store::files::FileRecord::get_file_id,
                &app::store::files::FileRecord::set_file_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::files::FileRecord::get_merchant_id,
                &app::store::files::FileRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::store::files::FileRecord::get_profile_id,
                &app::store::files::FileRecord::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_name",
                &app::store::files::FileRecord::get_file_name,
                &app::store::files::FileRecord::set_file_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_type",
                &app::store::files::FileRecord::get_file_type,
                &app::store::files::FileRecord::set_file_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "file_size",
                &app::store::files::FileRecord::get_file_size,
                &app::store::files::FileRecord::set_file_size,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "purpose",
                &app::store::files::FileRecord::get_purpose,
                &app::store::files::FileRecord::set_purpose,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "provider_file_id",
                &app::store::files::FileRecord::get_provider_file_id,
                &app::store::files::FileRecord::set_provider_file_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "upload_provider",
                &app::store::files::FileRecord::get_upload_provider,
                &app::store::files::FileRecord::set_upload_provider,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::files::FileRecord::get_status,
                &app::store::files::FileRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::files::FileRecord::get_created_at,
                &app::store::files::FileRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "files";
    }
};

export namespace app::store::cards_info {

/** @brief Persisted card information keyed by IIN. */
class CardInfoRecord
{
public:
    CardInfoRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_iin(std::string value)
    {
        m_iin = std::move(value);
    }

    [[nodiscard]] const std::string& get_iin() const noexcept
    {
        return m_iin;
    }

    void set_scheme(std::string value)
    {
        m_scheme = std::move(value);
    }

    [[nodiscard]] const std::string& get_scheme() const noexcept
    {
        return m_scheme;
    }

    void set_card_type(std::string value)
    {
        m_card_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_type() const noexcept
    {
        return m_card_type;
    }

    void set_bank_name(std::string value)
    {
        m_bank_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_bank_name() const noexcept
    {
        return m_bank_name;
    }

    void set_country(std::string value)
    {
        m_country = std::move(value);
    }

    [[nodiscard]] const std::string& get_country() const noexcept
    {
        return m_country;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    void set_updated_at(std::string value)
    {
        m_updated_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    std::int64_t m_id{};
    std::string m_iin{};
    std::string m_scheme{};
    std::string m_card_type{};
    std::string m_bank_name{};
    std::string m_country{};
    std::string m_status{};
    std::string m_created_at{};
    std::string m_updated_at{};
};

} // namespace app::store::cards_info

template<>
struct serde::Serializable<app::store::cards_info::CardInfoRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::cards_info::CardInfoRecord::get_id,
                &app::store::cards_info::CardInfoRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "iin",
                &app::store::cards_info::CardInfoRecord::get_iin,
                &app::store::cards_info::CardInfoRecord::set_iin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scheme",
                &app::store::cards_info::CardInfoRecord::get_scheme,
                &app::store::cards_info::CardInfoRecord::set_scheme,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::store::cards_info::CardInfoRecord::get_card_type,
                &app::store::cards_info::CardInfoRecord::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_name",
                &app::store::cards_info::CardInfoRecord::get_bank_name,
                &app::store::cards_info::CardInfoRecord::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country",
                &app::store::cards_info::CardInfoRecord::get_country,
                &app::store::cards_info::CardInfoRecord::set_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::cards_info::CardInfoRecord::get_status,
                &app::store::cards_info::CardInfoRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::cards_info::CardInfoRecord::get_created_at,
                &app::store::cards_info::CardInfoRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &app::store::cards_info::CardInfoRecord::get_updated_at,
                &app::store::cards_info::CardInfoRecord::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "cards_info";
    }
};

export namespace app::store::relay_records {

/** @brief Persisted relay record bridging payments and refunds. */
class RelayRecord
{
public:
    RelayRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_relay_id(std::string value)
    {
        m_relay_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_relay_id() const noexcept
    {
        return m_relay_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_profile_id(std::string value)
    {
        m_profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return m_profile_id;
    }

    void set_payment_id(std::string value)
    {
        m_payment_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_id() const noexcept
    {
        return m_payment_id;
    }

    void set_refund_id(std::string value)
    {
        m_refund_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_refund_id() const noexcept
    {
        return m_refund_id;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_payload(std::string value)
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_payload() const noexcept
    {
        return m_payload;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    std::int64_t m_id{};
    std::string m_relay_id{};
    std::string m_merchant_id{};
    std::string m_profile_id{};
    std::string m_payment_id{};
    std::string m_refund_id{};
    std::string m_status{};
    std::string m_payload{};
    std::string m_created_at{};
};

} // namespace app::store::relay_records

template<>
struct serde::Serializable<app::store::relay_records::RelayRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::relay_records::RelayRecord::get_id,
                &app::store::relay_records::RelayRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "relay_id",
                &app::store::relay_records::RelayRecord::get_relay_id,
                &app::store::relay_records::RelayRecord::set_relay_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::relay_records::RelayRecord::get_merchant_id,
                &app::store::relay_records::RelayRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::store::relay_records::RelayRecord::get_profile_id,
                &app::store::relay_records::RelayRecord::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::store::relay_records::RelayRecord::get_payment_id,
                &app::store::relay_records::RelayRecord::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_id",
                &app::store::relay_records::RelayRecord::get_refund_id,
                &app::store::relay_records::RelayRecord::set_refund_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::relay_records::RelayRecord::get_status,
                &app::store::relay_records::RelayRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payload",
                &app::store::relay_records::RelayRecord::get_payload,
                &app::store::relay_records::RelayRecord::set_payload,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::relay_records::RelayRecord::get_created_at,
                &app::store::relay_records::RelayRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "relay_records";
    }
};

export namespace app::store::tokenizations {

/** @brief Persisted tokenization record referencing an external vault id. */
class TokenizationRecord
{
public:
    TokenizationRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_token_id(std::string value)
    {
        m_token_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_token_id() const noexcept
    {
        return m_token_id;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_customer_id(std::string value)
    {
        m_customer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return m_customer_id;
    }

    void set_entity_id(std::string value)
    {
        m_entity_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_entity_id() const noexcept
    {
        return m_entity_id;
    }

    void set_vault_id(std::string value)
    {
        m_vault_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_vault_id() const noexcept
    {
        return m_vault_id;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    std::int64_t m_id{};
    std::string m_token_id{};
    std::string m_merchant_id{};
    std::string m_customer_id{};
    std::string m_entity_id{};
    std::string m_vault_id{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::tokenizations

template<>
struct serde::Serializable<app::store::tokenizations::TokenizationRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::tokenizations::TokenizationRecord::get_id,
                &app::store::tokenizations::TokenizationRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "token_id",
                &app::store::tokenizations::TokenizationRecord::get_token_id,
                &app::store::tokenizations::TokenizationRecord::set_token_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::tokenizations::TokenizationRecord::get_merchant_id,
                &app::store::tokenizations::TokenizationRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "customer_id",
                &app::store::tokenizations::TokenizationRecord::get_customer_id,
                &app::store::tokenizations::TokenizationRecord::set_customer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "entity_id",
                &app::store::tokenizations::TokenizationRecord::get_entity_id,
                &app::store::tokenizations::TokenizationRecord::set_entity_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "vault_id",
                &app::store::tokenizations::TokenizationRecord::get_vault_id,
                &app::store::tokenizations::TokenizationRecord::set_vault_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::tokenizations::TokenizationRecord::get_status,
                &app::store::tokenizations::TokenizationRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::tokenizations::TokenizationRecord::get_created_at,
                &app::store::tokenizations::TokenizationRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tokenizations";
    }
};

export namespace app::store::revenue_recovery_tokens {

/** @brief Persisted revenue-recovery token with card-data payload. */
class RevenueRecoveryTokenRecord
{
public:
    RevenueRecoveryTokenRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_token(std::string value)
    {
        m_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return m_token;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_payment_id(std::string value)
    {
        m_payment_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_id() const noexcept
    {
        return m_payment_id;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_payload(std::string value)
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_payload() const noexcept
    {
        return m_payload;
    }

    void set_expires_at(std::string value)
    {
        m_expires_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_expires_at() const noexcept
    {
        return m_expires_at;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    std::int64_t m_id{};
    std::string m_token{};
    std::string m_merchant_id{};
    std::string m_payment_id{};
    std::string m_status{};
    std::string m_payload{};
    std::string m_expires_at{};
    std::string m_created_at{};
};

} // namespace app::store::revenue_recovery_tokens

template<>
struct serde::Serializable<app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::get_id,
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "token",
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::get_token,
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::set_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::get_merchant_id,
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_id",
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::get_payment_id,
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::get_status,
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payload",
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::get_payload,
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::set_payload,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires_at",
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::get_expires_at,
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::set_expires_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::get_created_at,
                &app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "revenue_recovery_tokens";
    }
};

export namespace app::store::configs {

/** @brief Persisted key/value configuration record. */
class ConfigRecord
{
public:
    ConfigRecord() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_config_key(std::string value)
    {
        m_config_key = std::move(value);
    }

    [[nodiscard]] const std::string& get_config_key() const noexcept
    {
        return m_config_key;
    }

    void set_config_value(std::string value)
    {
        m_config_value = std::move(value);
    }

    [[nodiscard]] const std::string& get_config_value() const noexcept
    {
        return m_config_value;
    }

    void set_namespace(std::string value)
    {
        m_namespace = std::move(value);
    }

    [[nodiscard]] const std::string& get_namespace() const noexcept
    {
        return m_namespace;
    }

    void set_merchant_id(std::string value)
    {
        m_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return m_merchant_id;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_created_at(std::string value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return m_created_at;
    }

    std::int64_t m_id{};
    std::string m_config_key{};
    std::string m_config_value{};
    std::string m_namespace{};
    std::string m_merchant_id{};
    std::string m_status{};
    std::string m_created_at{};
};

} // namespace app::store::configs

template<>
struct serde::Serializable<app::store::configs::ConfigRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::store::configs::ConfigRecord::get_id,
                &app::store::configs::ConfigRecord::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "config_key",
                &app::store::configs::ConfigRecord::get_config_key,
                &app::store::configs::ConfigRecord::set_config_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "config_value",
                &app::store::configs::ConfigRecord::get_config_value,
                &app::store::configs::ConfigRecord::set_config_value,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "namespace",
                &app::store::configs::ConfigRecord::get_namespace,
                &app::store::configs::ConfigRecord::set_namespace,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::store::configs::ConfigRecord::get_merchant_id,
                &app::store::configs::ConfigRecord::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &app::store::configs::ConfigRecord::get_status,
                &app::store::configs::ConfigRecord::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::store::configs::ConfigRecord::get_created_at,
                &app::store::configs::ConfigRecord::set_created_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "configs";
    }
};

#ifdef CONGELADO_TEST
namespace app::store::ops_records::tests {

using namespace boost::ut;

suite<"app_store_ops_records"> ops_records_suite = []
{
    "process tracker fields and table"_test = []
    {
        process_trackers::ProcessTrackerRecord record{};
        record.set_task_id("task_1");
        record.set_status("scheduled");
        expect(record.get_task_id() == "task_1");
        expect(
            serde::Serializable<process_trackers::ProcessTrackerRecord>::table_name() ==
            "process_trackers"
        );
    };

    "file metadata fields and table"_test = []
    {
        files::FileRecord record{};
        record.set_file_id("file_1");
        record.set_file_size(12);
        expect(record.get_file_size() == 12);
        expect(serde::Serializable<files::FileRecord>::table_name() == "files");
    };

    "card info fields and table"_test = []
    {
        cards_info::CardInfoRecord record{};
        record.set_iin("411111");
        expect(record.get_iin() == "411111");
        expect(serde::Serializable<cards_info::CardInfoRecord>::table_name() == "cards_info");
    };

    "relay fields and table"_test = []
    {
        relay_records::RelayRecord record{};
        record.set_relay_id("relay_1");
        expect(record.get_relay_id() == "relay_1");
        expect(serde::Serializable<relay_records::RelayRecord>::table_name() == "relay_records");
    };

    "tokenization fields and table"_test = []
    {
        tokenizations::TokenizationRecord record{};
        record.set_token_id("tok_1");
        expect(record.get_token_id() == "tok_1");
        expect(
            serde::Serializable<tokenizations::TokenizationRecord>::table_name() == "tokenizations"
        );
    };

    "revenue recovery token fields and table"_test = []
    {
        revenue_recovery_tokens::RevenueRecoveryTokenRecord record{};
        record.set_token("rrt_1");
        expect(record.get_token() == "rrt_1");
        expect(
            serde::Serializable<
                revenue_recovery_tokens::RevenueRecoveryTokenRecord>::table_name() ==
            "revenue_recovery_tokens"
        );
    };

    "config fields and table"_test = []
    {
        configs::ConfigRecord record{};
        record.set_config_key("connector_onboarding.paypal");
        record.set_config_value("tracking");
        expect(record.get_config_key() == "connector_onboarding.paypal");
        expect(serde::Serializable<configs::ConfigRecord>::table_name() == "configs");
    };
};

} // namespace app::store::ops_records::tests
#endif
