module;
export module app_dto:revenue_recovery_data_backfill;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::revenue_recovery_data_backfill {

class AdditionalCardInfoValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class BackfillQuery
{
public:
    void set_cutoff_time(std::optional<std::string> value) { cutoff_time = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_cutoff_time() const noexcept { return cutoff_time; }
    std::optional<std::string> cutoff_time{};
};

class CardNetworkValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CsvParsingError
{
public:
    void set_row_number(std::size_t value) { row_number = std::move(value); }
    [[nodiscard]] const std::size_t& get_row_number() const noexcept { return row_number; }
    void set_error(std::string value) { error = std::move(value); }
    [[nodiscard]] const std::string& get_error() const noexcept { return error; }
    std::size_t row_number{};
    std::string error{};
};

class GlobalPaymentIdValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PaymentMethodTypeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class PrimitiveDateTimeValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class RedisDataResponse
{
public:
    void set_exists(bool value) { exists = std::move(value); }
    [[nodiscard]] const bool& get_exists() const noexcept { return exists; }
    void set_ttl_seconds(std::int64_t value) { ttl_seconds = std::move(value); }
    [[nodiscard]] const std::int64_t& get_ttl_seconds() const noexcept { return ttl_seconds; }
    void set_data(std::optional<serde::Value> value) { data = std::move(value); }
    [[nodiscard]] const std::optional<serde::Value>& get_data() const noexcept { return data; }
    bool exists{};
    std::int64_t ttl_seconds{};
    std::optional<serde::Value> data{};
};

enum class RedisKeyType
{
    Status,
    Tokens,
};

class RetryStatsMigrationRecord
{
public:
    void set_cluster_key(std::string value) { cluster_key = std::move(value); }
    [[nodiscard]] const std::string& get_cluster_key() const noexcept { return cluster_key; }
    void set_stats(std::string value) { stats = std::move(value); }
    [[nodiscard]] const std::string& get_stats() const noexcept { return stats; }
    std::string cluster_key{};
    std::string stats{};
};

class RevenueRecoveryDataBackfillResponse
{
public:
    void set_processed_records(std::size_t value) { processed_records = std::move(value); }
    [[nodiscard]] const std::size_t& get_processed_records() const noexcept { return processed_records; }
    void set_failed_records(std::size_t value) { failed_records = std::move(value); }
    [[nodiscard]] const std::size_t& get_failed_records() const noexcept { return failed_records; }
    std::size_t processed_records{};
    std::size_t failed_records{};
};

enum class ScheduledAtUpdate
{
    SetToNull,
    SetToDateTime,
};

class TempFileValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class UnlockStatusResponse
{
public:
    void set_unlocked(bool value) { unlocked = std::move(value); }
    [[nodiscard]] const bool& get_unlocked() const noexcept { return unlocked; }
    bool unlocked{};
};

class UpdateTokenStatusResponse
{
public:
    void set_updated(bool value) { updated = std::move(value); }
    [[nodiscard]] const bool& get_updated() const noexcept { return updated; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    bool updated{};
    std::string message{};
};

class AccountUpdateHistoryRecord
{
public:
    void set_old_token(std::string value) { old_token = std::move(value); }
    [[nodiscard]] const std::string& get_old_token() const noexcept { return old_token; }
    void set_new_token(std::string value) { new_token = std::move(value); }
    [[nodiscard]] const std::string& get_new_token() const noexcept { return new_token; }
    void set_updated_at(PrimitiveDateTimeValue value) { updated_at = std::move(value); }
    [[nodiscard]] const PrimitiveDateTimeValue& get_updated_at() const noexcept { return updated_at; }
    void set_old_token_info(std::optional<AdditionalCardInfoValue> value) { old_token_info = std::move(value); }
    [[nodiscard]] const std::optional<AdditionalCardInfoValue>& get_old_token_info() const noexcept { return old_token_info; }
    void set_new_token_info(std::optional<AdditionalCardInfoValue> value) { new_token_info = std::move(value); }
    [[nodiscard]] const std::optional<AdditionalCardInfoValue>& get_new_token_info() const noexcept { return new_token_info; }
    std::string old_token{};
    std::string new_token{};
    PrimitiveDateTimeValue updated_at{};
    std::optional<AdditionalCardInfoValue> old_token_info{};
    std::optional<AdditionalCardInfoValue> new_token_info{};
};

enum class BackfillError
{
    InvalidCardType,
    DatabaseError,
    RedisError,
    CsvParsingError,
    FileProcessingError,
};

class GetRedisDataQuery
{
public:
    void set_key_type(RedisKeyType value) { key_type = std::move(value); }
    [[nodiscard]] const RedisKeyType& get_key_type() const noexcept { return key_type; }
    RedisKeyType key_type{};
};

class RetryStatsMigrationCsvResult
{
public:
    void set_records(std::vector<std::pair<std::size_t, RetryStatsMigrationRecord>> value) { records = std::move(value); }
    [[nodiscard]] const std::vector<std::pair<std::size_t, RetryStatsMigrationRecord>>& get_records() const noexcept { return records; }
    std::vector<std::pair<std::size_t, RetryStatsMigrationRecord>> records{};
};

class RetryStatsMigrationForm
{
public:
    void set_file(TempFileValue value) { file = std::move(value); }
    [[nodiscard]] const TempFileValue& get_file() const noexcept { return file; }
    TempFileValue file{};
};

class RetryStatsMigrationResponse
{
public:
    void set_processed_records(std::size_t value) { processed_records = std::move(value); }
    [[nodiscard]] const std::size_t& get_processed_records() const noexcept { return processed_records; }
    void set_failed_records(std::size_t value) { failed_records = std::move(value); }
    [[nodiscard]] const std::size_t& get_failed_records() const noexcept { return failed_records; }
    void set_row_errors(std::vector<CsvParsingError> value) { row_errors = std::move(value); }
    [[nodiscard]] const std::vector<CsvParsingError>& get_row_errors() const noexcept { return row_errors; }
    std::size_t processed_records{};
    std::size_t failed_records{};
    std::vector<CsvParsingError> row_errors{};
};

class RevenueRecoveryDataBackfillForm
{
public:
    void set_file(TempFileValue value) { file = std::move(value); }
    [[nodiscard]] const TempFileValue& get_file() const noexcept { return file; }
    TempFileValue file{};
};

class UnlockStatusRequest
{
public:
    void set_connector_customer_id(std::string value) { connector_customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_connector_customer_id() const noexcept { return connector_customer_id; }
    void set_payment_intent_id(GlobalPaymentIdValue value) { payment_intent_id = std::move(value); }
    [[nodiscard]] const GlobalPaymentIdValue& get_payment_intent_id() const noexcept { return payment_intent_id; }
    std::string connector_customer_id{};
    GlobalPaymentIdValue payment_intent_id{};
};

class UpdateTokenStatusRequest
{
public:
    void set_connector_customer_id(std::string value) { connector_customer_id = std::move(value); }
    [[nodiscard]] const std::string& get_connector_customer_id() const noexcept { return connector_customer_id; }
    void set_payment_processor_token(std::string value) { payment_processor_token = std::move(value); }
    [[nodiscard]] const std::string& get_payment_processor_token() const noexcept { return payment_processor_token; }
    void set_scheduled_at(std::optional<ScheduledAtUpdate> value) { scheduled_at = std::move(value); }
    [[nodiscard]] const std::optional<ScheduledAtUpdate>& get_scheduled_at() const noexcept { return scheduled_at; }
    void set_is_hard_decline(std::optional<bool> value) { is_hard_decline = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_hard_decline() const noexcept { return is_hard_decline; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    std::string connector_customer_id{};
    std::string payment_processor_token{};
    std::optional<ScheduledAtUpdate> scheduled_at{};
    std::optional<bool> is_hard_decline{};
    std::optional<std::string> error_code{};
};

class ComprehensiveCardData
{
public:
    void set_card_type(std::optional<std::string> value) { card_type = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_card_type() const noexcept { return card_type; }
    void set_card_exp_month(std::optional<std::string> value) { card_exp_month = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_card_exp_month() const noexcept { return card_exp_month; }
    void set_card_exp_year(std::optional<std::string> value) { card_exp_year = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_card_exp_year() const noexcept { return card_exp_year; }
    void set_card_network(std::optional<CardNetworkValue> value) { card_network = std::move(value); }
    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept { return card_network; }
    void set_card_issuer(std::optional<std::string> value) { card_issuer = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_card_issuer() const noexcept { return card_issuer; }
    void set_card_issuing_country(std::optional<std::string> value) { card_issuing_country = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_card_issuing_country() const noexcept { return card_issuing_country; }
    void set_daily_retry_history(std::optional<std::unordered_map<std::string, std::int32_t>> value) { daily_retry_history = std::move(value); }
    [[nodiscard]] const std::optional<std::unordered_map<std::string, std::int32_t>>& get_daily_retry_history() const noexcept { return daily_retry_history; }
    void set_is_active(std::optional<bool> value) { is_active = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_active() const noexcept { return is_active; }
    void set_account_update_history(std::optional<std::vector<AccountUpdateHistoryRecord>> value) { account_update_history = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<AccountUpdateHistoryRecord>>& get_account_update_history() const noexcept { return account_update_history; }
    std::optional<std::string> card_type{};
    std::optional<std::string> card_exp_month{};
    std::optional<std::string> card_exp_year{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<std::string> card_issuer{};
    std::optional<std::string> card_issuing_country{};
    std::optional<std::unordered_map<std::string, std::int32_t>> daily_retry_history{};
    std::optional<bool> is_active{};
    std::optional<std::vector<AccountUpdateHistoryRecord>> account_update_history{};
};

class RevenueRecoveryBackfillRequest
{
public:
    void set_bin_number(std::optional<std::string> value) { bin_number = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bin_number() const noexcept { return bin_number; }
    void set_customer_id_resp(std::string value) { customer_id_resp = std::move(value); }
    [[nodiscard]] const std::string& get_customer_id_resp() const noexcept { return customer_id_resp; }
    void set_connector_payment_id(std::optional<std::string> value) { connector_payment_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_connector_payment_id() const noexcept { return connector_payment_id; }
    void set_token(std::optional<std::string> value) { token = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_token() const noexcept { return token; }
    void set_exp_date(std::optional<std::string> value) { exp_date = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_exp_date() const noexcept { return exp_date; }
    void set_card_network(std::optional<CardNetworkValue> value) { card_network = std::move(value); }
    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept { return card_network; }
    void set_payment_method_sub_type(std::optional<PaymentMethodTypeValue> value) { payment_method_sub_type = std::move(value); }
    [[nodiscard]] const std::optional<PaymentMethodTypeValue>& get_payment_method_sub_type() const noexcept { return payment_method_sub_type; }
    void set_clean_bank_name(std::optional<std::string> value) { clean_bank_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_clean_bank_name() const noexcept { return clean_bank_name; }
    void set_country_name(std::optional<std::string> value) { country_name = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_country_name() const noexcept { return country_name; }
    void set_daily_retry_history(std::optional<std::string> value) { daily_retry_history = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_daily_retry_history() const noexcept { return daily_retry_history; }
    void set_is_active(std::optional<bool> value) { is_active = std::move(value); }
    [[nodiscard]] const std::optional<bool>& get_is_active() const noexcept { return is_active; }
    void set_account_update_history(std::optional<std::vector<AccountUpdateHistoryRecord>> value) { account_update_history = std::move(value); }
    [[nodiscard]] const std::optional<std::vector<AccountUpdateHistoryRecord>>& get_account_update_history() const noexcept { return account_update_history; }
    std::optional<std::string> bin_number{};
    std::string customer_id_resp{};
    std::optional<std::string> connector_payment_id{};
    std::optional<std::string> token{};
    std::optional<std::string> exp_date{};
    std::optional<CardNetworkValue> card_network{};
    std::optional<PaymentMethodTypeValue> payment_method_sub_type{};
    std::optional<std::string> clean_bank_name{};
    std::optional<std::string> country_name{};
    std::optional<std::string> daily_retry_history{};
    std::optional<bool> is_active{};
    std::optional<std::vector<AccountUpdateHistoryRecord>> account_update_history{};
};

class CsvParsingResult
{
public:
    void set_records(std::vector<RevenueRecoveryBackfillRequest> value) { records = std::move(value); }
    [[nodiscard]] const std::vector<RevenueRecoveryBackfillRequest>& get_records() const noexcept { return records; }
    void set_failed_records(std::vector<CsvParsingError> value) { failed_records = std::move(value); }
    [[nodiscard]] const std::vector<CsvParsingError>& get_failed_records() const noexcept { return failed_records; }
    std::vector<RevenueRecoveryBackfillRequest> records{};
    std::vector<CsvParsingError> failed_records{};
};
} // namespace app::dto::revenue_recovery_data_backfill

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"old_token", &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::get_old_token, &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::set_old_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"new_token", &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::get_new_token, &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::set_new_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::get_updated_at, &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::set_updated_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"old_token_info", &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::get_old_token_info, &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::set_old_token_info, serde::FieldOptions{}>{},
            serde::FieldDesc<"new_token_info", &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::get_new_token_info, &app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord::set_new_token_info, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "account_update_history_record"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"bin_number", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_bin_number, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_bin_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id_resp", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_customer_id_resp, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_customer_id_resp, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_payment_id", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_connector_payment_id, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_connector_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"token", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_token, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"exp_date", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_exp_date, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_exp_date, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_network", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_card_network, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_card_network, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method_sub_type", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_payment_method_sub_type, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_payment_method_sub_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"clean_bank_name", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_clean_bank_name, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_clean_bank_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"country_name", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_country_name, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_country_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"daily_retry_history", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_daily_retry_history, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_daily_retry_history, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_active", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_is_active, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_is_active, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_update_history", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::get_account_update_history, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest::set_account_update_history, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "revenue_recovery_backfill_request"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::CsvParsingError>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"row_number", &app::dto::revenue_recovery_data_backfill::CsvParsingError::get_row_number, &app::dto::revenue_recovery_data_backfill::CsvParsingError::set_row_number, serde::FieldOptions{}>{},
            serde::FieldDesc<"error", &app::dto::revenue_recovery_data_backfill::CsvParsingError::get_error, &app::dto::revenue_recovery_data_backfill::CsvParsingError::set_error, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "csv_parsing_error"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::CsvParsingResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"records", &app::dto::revenue_recovery_data_backfill::CsvParsingResult::get_records, &app::dto::revenue_recovery_data_backfill::CsvParsingResult::set_records, serde::FieldOptions{}>{},
            serde::FieldDesc<"failed_records", &app::dto::revenue_recovery_data_backfill::CsvParsingResult::get_failed_records, &app::dto::revenue_recovery_data_backfill::CsvParsingResult::set_failed_records, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "csv_parsing_result"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::RetryStatsMigrationRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"cluster_key", &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationRecord::get_cluster_key, &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationRecord::set_cluster_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"stats", &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationRecord::get_stats, &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationRecord::set_stats, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "retry_stats_migration_record"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"records", &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult::get_records, &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult::set_records, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "retry_stats_migration_csv_result"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"processed_records", &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse::get_processed_records, &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse::set_processed_records, serde::FieldOptions{}>{},
            serde::FieldDesc<"failed_records", &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse::get_failed_records, &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse::set_failed_records, serde::FieldOptions{}>{},
            serde::FieldDesc<"row_errors", &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse::get_row_errors, &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse::set_row_errors, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "retry_stats_migration_response"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::ComprehensiveCardData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"card_type", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_card_type, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_card_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_exp_month", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_card_exp_month, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_card_exp_month, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_exp_year", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_card_exp_year, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_card_exp_year, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_network", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_card_network, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_card_network, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_issuer", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_card_issuer, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_card_issuer, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_issuing_country", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_card_issuing_country, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_card_issuing_country, serde::FieldOptions{}>{},
            serde::FieldDesc<"daily_retry_history", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_daily_retry_history, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_daily_retry_history, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_active", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_is_active, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_is_active, serde::FieldOptions{}>{},
            serde::FieldDesc<"account_update_history", &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::get_account_update_history, &app::dto::revenue_recovery_data_backfill::ComprehensiveCardData::set_account_update_history, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "comprehensive_card_data"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::UnlockStatusResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"unlocked", &app::dto::revenue_recovery_data_backfill::UnlockStatusResponse::get_unlocked, &app::dto::revenue_recovery_data_backfill::UnlockStatusResponse::set_unlocked, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "unlock_status_response"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::UnlockStatusRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_customer_id", &app::dto::revenue_recovery_data_backfill::UnlockStatusRequest::get_connector_customer_id, &app::dto::revenue_recovery_data_backfill::UnlockStatusRequest::set_connector_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_intent_id", &app::dto::revenue_recovery_data_backfill::UnlockStatusRequest::get_payment_intent_id, &app::dto::revenue_recovery_data_backfill::UnlockStatusRequest::set_payment_intent_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "unlock_status_request"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"processed_records", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse::get_processed_records, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse::set_processed_records, serde::FieldOptions{}>{},
            serde::FieldDesc<"failed_records", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse::get_failed_records, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse::set_failed_records, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "revenue_recovery_data_backfill_response"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::BackfillQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"cutoff_time", &app::dto::revenue_recovery_data_backfill::BackfillQuery::get_cutoff_time, &app::dto::revenue_recovery_data_backfill::BackfillQuery::set_cutoff_time, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "backfill_query"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::GetRedisDataQuery>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"key_type", &app::dto::revenue_recovery_data_backfill::GetRedisDataQuery::get_key_type, &app::dto::revenue_recovery_data_backfill::GetRedisDataQuery::set_key_type, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "get_redis_data_query"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::RedisDataResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"exists", &app::dto::revenue_recovery_data_backfill::RedisDataResponse::get_exists, &app::dto::revenue_recovery_data_backfill::RedisDataResponse::set_exists, serde::FieldOptions{}>{},
            serde::FieldDesc<"ttl_seconds", &app::dto::revenue_recovery_data_backfill::RedisDataResponse::get_ttl_seconds, &app::dto::revenue_recovery_data_backfill::RedisDataResponse::set_ttl_seconds, serde::FieldOptions{}>{},
            serde::FieldDesc<"data", &app::dto::revenue_recovery_data_backfill::RedisDataResponse::get_data, &app::dto::revenue_recovery_data_backfill::RedisDataResponse::set_data, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "redis_data_response"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"connector_customer_id", &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::get_connector_customer_id, &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::set_connector_customer_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_processor_token", &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::get_payment_processor_token, &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::set_payment_processor_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"scheduled_at", &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::get_scheduled_at, &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::set_scheduled_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_hard_decline", &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::get_is_hard_decline, &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::set_is_hard_decline, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::get_error_code, &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest::set_error_code, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "update_token_status_request"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"updated", &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse::get_updated, &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse::set_updated, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse::get_message, &app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse::set_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "update_token_status_response"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillForm>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"file", &app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillForm::get_file, &app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillForm::set_file, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "revenue_recovery_data_backfill_form"; }
};

template<>
struct serde::Serializable<app::dto::revenue_recovery_data_backfill::RetryStatsMigrationForm>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"file", &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationForm::get_file, &app::dto::revenue_recovery_data_backfill::RetryStatsMigrationForm::set_file, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "retry_stats_migration_form"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::revenue_recovery_data_backfill::tests {
using namespace boost::ut;
suite<"app_dto_revenue_recovery_data_backfill"> app_dto_revenue_recovery_data_backfill_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest>::fields())> > 0);
    };
};
}
#endif
