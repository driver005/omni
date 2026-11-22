module;
export module app_handlers:revenue_recovery_data_backfill;

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
import app_metrics;
import app_store;
import app_crypto;
import app_locking;
import app_services;

export namespace app::handlers::revenue_recovery_data_backfill {

namespace detail {

/**
 * @brief Local in-process substitute for `time::PrimitiveDateTime`.
 *
 * Upstream parses retry-history keys with two `format_description!` layouts
 * (core/revenue_recovery_data_backfill.rs:343-358) and the cutoff query with
 * `Iso8601::DEFAULT` (routes/revenue_recovery_data_backfill.rs:29-46). The C++
 * standard library has no civil-calendar type, so this value type provides the
 * same parse / compare / serialize surface and renders keys in the exact
 * `YYYY-MM-DD HH:MM:SS.ffffff` shape used by upstream's Redis token status.
 */
struct PrimitiveDateTime
{
    std::int64_t year{1970};
    std::uint32_t month{1};
    std::uint32_t day{1};
    std::uint32_t hour{0};
    std::uint32_t minute{0};
    std::uint32_t second{0};
    std::uint32_t micro{0};

    [[nodiscard]] auto operator<=>(const PrimitiveDateTime&) const noexcept = default;
    [[nodiscard]] bool operator==(const PrimitiveDateTime&) const noexcept = default;

    [[nodiscard]] static std::optional<std::uint32_t>
    read_uint(std::string_view text, std::size_t at, std::size_t count) noexcept
    {
        if (at + count > text.size()) {
            return std::nullopt;
        }
        std::uint32_t value{0};
        for (std::size_t i = 0; i < count; ++i) {
            const char ch = text[at + i];
            if (ch < '0' || ch > '9') {
                return std::nullopt;
            }
            value = value * std::uint32_t{10} + static_cast<std::uint32_t>(ch - '0');
        }
        return value;
    }

    /** Full `[year]-[month]-[day] [hour]:[minute]:[second].[subsecond]`, then date-only. */
    [[nodiscard]] static std::optional<PrimitiveDateTime> parse(std::string_view text) noexcept
    {
        const auto year = read_uint(text, 0, 4);
        if (!year || text.size() < std::size_t{10} || text[4] != '-' || text[7] != '-') {
            return std::nullopt;
        }
        const auto month = read_uint(text, 5, 2);
        const auto day = read_uint(text, 8, 2);
        if (!month || !day || *month < std::uint32_t{1} || *month > std::uint32_t{12}
            || *day < std::uint32_t{1} || *day > std::uint32_t{31}) {
            return std::nullopt;
        }
        PrimitiveDateTime out{};
        out.year = static_cast<std::int64_t>(*year);
        out.month = *month;
        out.day = *day;
        if (text.size() == std::size_t{10}) {
            return out; // Date::parse -> Time::MIDNIGHT fallback.
        }
        if (text.size() < std::size_t{19} || (text[10] != ' ' && text[10] != 'T')
            || text[13] != ':' || text[16] != ':') {
            return std::nullopt;
        }
        const auto hour = read_uint(text, 11, 2);
        const auto minute = read_uint(text, 14, 2);
        const auto second = read_uint(text, 17, 2);
        if (!hour || !minute || !second || *hour > std::uint32_t{23}
            || *minute > std::uint32_t{59} || *second > std::uint32_t{59}) {
            return std::nullopt;
        }
        out.hour = *hour;
        out.minute = *minute;
        out.second = *second;
        if (text.size() > std::size_t{19} && text[19] == '.') {
            std::uint32_t digits{0};
            std::size_t count{0};
            for (std::size_t i = 20; i < text.size() && count < std::size_t{6}; ++i) {
                if (text[i] < '0' || text[i] > '9') {
                    break;
                }
                digits = digits * std::uint32_t{10} + static_cast<std::uint32_t>(text[i] - '0');
                ++count;
            }
            for (std::size_t i = count; i < std::size_t{6}; ++i) {
                digits *= std::uint32_t{10};
            }
            out.micro = digits;
        }
        return out;
    }

    /** `time::format_description::well_known::Iso8601::DEFAULT` for BackfillQuery.cutoff_time. */
    [[nodiscard]] static std::optional<PrimitiveDateTime> parse_iso8601(std::string_view text) noexcept
    {
        const auto year = read_uint(text, 0, 4);
        if (!year || text.size() < std::size_t{19} || text[4] != '-' || text[7] != '-'
            || (text[10] != 'T' && text[10] != ' ')) {
            return std::nullopt;
        }
        const auto month = read_uint(text, 5, 2);
        const auto day = read_uint(text, 8, 2);
        const auto hour = read_uint(text, 11, 2);
        const auto minute = read_uint(text, 14, 2);
        const auto second = read_uint(text, 17, 2);
        if (!month || !day || !hour || !minute || !second || *month < std::uint32_t{1}
            || *month > std::uint32_t{12} || *day < std::uint32_t{1}
            || *day > std::uint32_t{31} || *hour > std::uint32_t{23}
            || *minute > std::uint32_t{59} || *second > std::uint32_t{59}) {
            return std::nullopt;
        }
        auto out = PrimitiveDateTime{};
        out.year = static_cast<std::int64_t>(*year);
        out.month = *month;
        out.day = *day;
        out.hour = *hour;
        out.minute = *minute;
        out.second = *second;
        if (text.size() > std::size_t{19} && text[19] == '.') {
            std::uint32_t digits{0};
            std::size_t count{0};
            for (std::size_t i = 20; i < text.size() && count < std::size_t{6}; ++i) {
                if (text[i] < '0' || text[i] > '9') {
                    break;
                }
                digits = digits * std::uint32_t{10} + static_cast<std::uint32_t>(text[i] - '0');
                ++count;
            }
            for (std::size_t i = count; i < std::size_t{6}; ++i) {
                digits *= std::uint32_t{10};
            }
            out.micro = digits;
        }
        return out;
    }

    /** `date_time::now()` stand-in: UTC civil time from system_clock. */
    [[nodiscard]] static PrimitiveDateTime now() noexcept
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
                             )
                                 .count();
        const std::int64_t days = seconds / std::int64_t{86400};
        const std::int64_t seconds_of_day = seconds % std::int64_t{86400};
        // Howard Hinnant's civil_from_days.
        const std::int64_t z = days + std::int64_t{719468};
        const std::int64_t era = (z >= 0 ? z : z - std::int64_t{146096}) / std::int64_t{146097};
        const std::int64_t doe = z - era * std::int64_t{146097};
        const std::int64_t yoe =
            (doe - doe / std::int64_t{1460} + doe / std::int64_t{36524}
             - doe / std::int64_t{146096})
            / std::int64_t{365};
        std::int64_t year = yoe + era * std::int64_t{400};
        const std::int64_t doy =
            doe - (std::int64_t{365} * yoe + yoe / std::int64_t{4} - yoe / std::int64_t{100});
        const std::int64_t mp = (std::int64_t{5} * doy + std::int64_t{2}) / std::int64_t{153};
        const std::int64_t day = doy - (std::int64_t{153} * mp + std::int64_t{2}) / std::int64_t{5}
                                 + std::int64_t{1};
        const std::int64_t month = mp + (mp < std::int64_t{10} ? std::int64_t{3}
                                                                : -std::int64_t{9});
        year += (month <= std::int64_t{2}) ? std::int64_t{1} : std::int64_t{0};
        PrimitiveDateTime out{};
        out.year = year;
        out.month = static_cast<std::uint32_t>(month);
        out.day = static_cast<std::uint32_t>(day);
        out.hour = static_cast<std::uint32_t>(seconds_of_day / std::int64_t{3600});
        out.minute = static_cast<std::uint32_t>((seconds_of_day % std::int64_t{3600}) / std::int64_t{60});
        out.second = static_cast<std::uint32_t>(seconds_of_day % std::int64_t{60});
        return out;
    }

    /** Upstream Redis-key / CSV-key datetime layout. */
    [[nodiscard]] std::string storage_string() const
    {
        return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:06}", year, month, day, hour,
                           minute, second, micro);
    }
};

/** `types/storage/revenue_recovery_redis_operation.rs:21-31`. */
struct PaymentProcessorTokenDetails
{
#define RRDB_FIELD(name, ...)                                                                    \
    __VA_ARGS__ name{};                                                                          \
    void set_##name(__VA_ARGS__ value) { name = std::move(value); }                              \
    [[nodiscard]] const __VA_ARGS__& get_##name() const noexcept { return name; }
    RRDB_FIELD(payment_processor_token, std::string)
    RRDB_FIELD(expiry_month, std::optional<std::string>)
    RRDB_FIELD(expiry_year, std::optional<std::string>)
    RRDB_FIELD(card_issuer, std::optional<std::string>)
    RRDB_FIELD(last_four_digits, std::optional<std::string>)
    RRDB_FIELD(card_network, std::optional<std::string>)
    RRDB_FIELD(card_type, std::optional<std::string>)
    RRDB_FIELD(card_isin, std::optional<std::string>)
#undef RRDB_FIELD
};

/** `types/storage/revenue_recovery_redis_operation.rs:34-57`. */
struct PaymentProcessorTokenStatus
{
    using History = std::vector<
        app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord>;
#define RRDB_FIELD(name, ...)                                                                    \
    __VA_ARGS__ name{};                                                                          \
    void set_##name(__VA_ARGS__ value) { name = std::move(value); }                              \
    [[nodiscard]] const __VA_ARGS__& get_##name() const noexcept { return name; }
    RRDB_FIELD(payment_processor_token_details, PaymentProcessorTokenDetails)
    RRDB_FIELD(inserted_by_attempt_id, std::string)
    RRDB_FIELD(error_code, std::optional<std::string>)
    RRDB_FIELD(daily_retry_history, std::unordered_map<std::string, std::int32_t>)
    RRDB_FIELD(scheduled_at, std::optional<std::string>)
    RRDB_FIELD(is_hard_decline, std::optional<bool>)
    RRDB_FIELD(modified_at, std::optional<std::string>)
    RRDB_FIELD(is_active, std::optional<bool>)
    RRDB_FIELD(account_update_history, std::optional<History>)
    RRDB_FIELD(decision_threshold, std::optional<double>)
#undef RRDB_FIELD
};

/**
 * @brief Local, in-process substitute for `RedisTokenManager`'s Redis connection.
 *
 * The cache entry is the Redis key; the value is the raw string (Status) or a
 * JSON object of token-id -> serialized token status (the Redis hash for
 * Tokens). Keys and TTLs mirror
 * `types/storage/revenue_recovery_redis_operation.rs:139-145, 162-170, 384-395`.
 */
class LocalRedis
{
public:
    explicit LocalRedis(core::cache::Cache& cache) noexcept : m_cache{cache} {}

    [[nodiscard]] static std::string status_key(std::string_view customer_id)
    {
        return "customer:" + std::string{customer_id} + ":status";
    }

    [[nodiscard]] static std::string tokens_key(std::string_view customer_id)
    {
        return "customer:" + std::string{customer_id} + ":tokens";
    }

    [[nodiscard]] static std::string scoped(std::string_view redis_key)
    {
        return "revenue_recovery_redis.redis:" + std::string{redis_key};
    }

    /** SETNX with expiry: true only when the key was absent. */
    bool set_key_if_not_exists_with_expiry(
        std::string_view redis_key,
        std::string_view value,
        std::int64_t ttl_seconds
    )
    {
        const std::string key = scoped(redis_key);
        if (m_cache.get(key).hit) {
            return false;
        }
        m_cache.put(key, core::cache::to_cache_value(std::string{value}),
                    core::cache::Ttl{}.seconds(ttl_seconds));
        return true;
    }

    [[nodiscard]] std::optional<std::string> get_key(std::string_view redis_key) const
    {
        const auto result = m_cache.get(scoped(redis_key));
        if (!result.hit || core::cache::IsMiss{}(result.value)) {
            return std::nullopt;
        }
        return core::cache::value_as<std::string>(result.value, std::string{});
    }

    bool delete_key(std::string_view redis_key)
    {
        return m_cache.erase(scoped(redis_key));
    }

    [[nodiscard]] std::unordered_map<std::string, std::string>
    get_hash_fields(std::string_view redis_key) const
    {
        std::unordered_map<std::string, std::string> out{};
        const auto raw = get_key(redis_key);
        if (!raw) {
            return out; // HGETALL on a missing key -> empty map.
        }
        const auto decoded = serde::Ser::decode_generic("application/json", *raw);
        if (!decoded) {
            return out;
        }
        const auto* object = std::get_if<serde::Value::Object>(&decoded->get());
        if (object == nullptr) {
            return out;
        }
        for (const auto& [token, value] : *object) {
            if (const auto* text = std::get_if<std::string>(&value.get())) {
                out.emplace(token, *text);
            }
        }
        return out;
    }

    void set_hash_fields(
        std::string_view redis_key,
        const std::unordered_map<std::string, std::string>& fields,
        std::int64_t ttl_seconds
    )
    {
        serde::Value::Object object{};
        for (const auto& [token, status] : fields) {
            object[token] = serde::Value{status};
        }
        serde::Value document{std::move(object)};
        m_cache.put(scoped(redis_key),
                    core::cache::to_cache_value(serde::Ser::encode_json(document)),
                    core::cache::Ttl{}.seconds(ttl_seconds));
    }

private:
    core::cache::Cache& m_cache;
};

/** `dow`/`dom`/`hod` slot counters of `StatsDocument` (retry_stats_document.rs:1-43). */
struct SlotCounter
{
    std::uint64_t n{0};
    std::uint64_t k{0};
};

struct StatsDocument
{
    std::vector<SlotCounter> dow{};
    std::vector<SlotCounter> dom{};
    std::vector<SlotCounter> hod{};
};

} // namespace detail
} // namespace app::handlers::revenue_recovery_data_backfill

// rfl reflection for the local token-status storage record (declared before the
// handler so encode/decode instantiation sees the exact upstream field names).
namespace serde {
template<>
struct Serializable<
    app::handlers::revenue_recovery_data_backfill::detail::PaymentProcessorTokenDetails>
{
    static constexpr auto fields()
    {
        using T = app::handlers::revenue_recovery_data_backfill::detail::PaymentProcessorTokenDetails;
        return std::tuple{
            serde::FieldDesc<"payment_processor_token", &T::get_payment_processor_token,
                             &T::set_payment_processor_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_month", &T::get_expiry_month, &T::set_expiry_month,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_year", &T::get_expiry_year, &T::set_expiry_year,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"card_issuer", &T::get_card_issuer, &T::set_card_issuer,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"last_four_digits", &T::get_last_four_digits,
                             &T::set_last_four_digits, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_network", &T::get_card_network, &T::set_card_network,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"card_type", &T::get_card_type, &T::set_card_type,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"card_isin", &T::get_card_isin, &T::set_card_isin,
                             serde::FieldOptions{}>{},
        };
    }
    static constexpr std::string_view table_name() { return "payment_processor_token_details"; }
};

template<>
struct ::serde::Serializable<
    app::handlers::revenue_recovery_data_backfill::detail::PaymentProcessorTokenStatus>
{
    static constexpr auto fields()
    {
        using T = app::handlers::revenue_recovery_data_backfill::detail::PaymentProcessorTokenStatus;
        return std::tuple{
            serde::FieldDesc<"payment_processor_token_details",
                             &T::get_payment_processor_token_details,
                             &T::set_payment_processor_token_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"inserted_by_attempt_id", &T::get_inserted_by_attempt_id,
                             &T::set_inserted_by_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &T::get_error_code, &T::set_error_code,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"daily_retry_history", &T::get_daily_retry_history,
                             &T::set_daily_retry_history, serde::FieldOptions{}>{},
            serde::FieldDesc<"scheduled_at", &T::get_scheduled_at, &T::set_scheduled_at,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"is_hard_decline", &T::get_is_hard_decline, &T::set_is_hard_decline,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"modified_at", &T::get_modified_at, &T::set_modified_at,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"is_active", &T::get_is_active, &T::set_is_active,
                             serde::FieldOptions{}>{},
            serde::FieldDesc<"account_update_history", &T::get_account_update_history,
                             &T::set_account_update_history, serde::FieldOptions{}>{},
            serde::FieldDesc<"decision_threshold", &T::get_decision_threshold,
                             &T::set_decision_threshold, serde::FieldOptions{}>{},
        };
    }
    static constexpr std::string_view table_name() { return "payment_processor_token_status"; }
};
} // namespace serde

namespace app::handlers::revenue_recovery_data_backfill {

export class RevenueRecoveryDataBackfillRouteHandlers
{
public:
    explicit RevenueRecoveryDataBackfillRouteHandlers(::app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:19-73 ->
     * core/revenue_recovery_data_backfill.rs:23-64.
     */
    void handle_revenue_recovery_data_backfill(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream parses cutoff_time and the multipart form BEFORE server_wrap
        // applies V2AdminApiAuth (routes:29-71); keep that order so malformed input
        // is rejected by the route regardless of the caller's permissions.
        const auto query = ::app::http::RequestQuery{}(req);
        const auto cutoff_time = query.get("cutoff_time");
        std::optional<detail::PrimitiveDateTime> cutoff_datetime{};
        if (cutoff_time && !cutoff_time->empty()) {
            cutoff_datetime = detail::PrimitiveDateTime::parse_iso8601(*cutoff_time);
            if (!cutoff_datetime) {
                // routes:42-45 builds {"error": format!("Invalid datetime format: {}. Use
                // ISO8601: 2024-01-15T10:30:00", err)}; time's parse error is the substituted
                // detail here (the local date type has no error enum).
                send_bad_request(
                    res, send,
                    "Invalid datetime format: invalid datetime format. Use ISO8601: 2024-01-15T10:30:00"
                );
                return;
            }
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                                      core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = ::app::dto::revenue_recovery_data_backfill::CsvParsingResult;
        std::optional<Input> parsed{};
        std::string parse_error{};
        if (is_json_body(body)) {
            auto decoded = serde::Ser::deserialize<Input>("application/json", body);
            if (!decoded) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                                          core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            parsed = std::move(*decoded);
        } else {
            // Local substitute for MultipartForm<RevenueRecoveryDataBackfillForm>:
            // validate_and_get_records_with_errors (api_models:298-331), run in-process.
            parsed = parse_backfill_csv(unwrap_multipart_file(body), parse_error);
            if (!parsed) {
                send_bad_request(res, send,
                                 "CSV parsing error: " + (parse_error.empty() ? "invalid CSV file"
                                                                              : parse_error));
                return;
            }
        }

        // server_wrap / V2AdminApiAuth is applied after route-level parsing (routes:57-71)
        // but before the core function mutates any token.
        if (const auto auth_error = authorize(req, "revenue_recovery_data_backfill")) {
            send_error(res, send, *auth_error);
            return;
        }

        std::size_t processed_records{0};
        std::size_t failed_records{0};
        for (const auto& record : parsed->get_records()) {
            // core:32-49: process_payment_method_record; Ok -> processed, Err -> failed.
            if (process_payment_method_record(record, cutoff_datetime)) {
                ++processed_records;
            } else {
                ++failed_records;
            }
        }
        // api_models:69-73: rows rejected by the CSV parser are already failed records.
        failed_records += parsed->get_failed_records().size();

        record_flow("RecoveryDataBackfill", "revenue_recovery_data_backfill", processed_records,
                    failed_records);

        using Response = ::app::dto::revenue_recovery_data_backfill::RevenueRecoveryDataBackfillResponse;
        Response response;
        response.set_processed_records(processed_records);
        response.set_failed_records(failed_records);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:75-104 ->
     * core/revenue_recovery/retry_stats/migration.rs:21-130.
     */
    void handle_revenue_recovery_retry_stats_migration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream validates the multipart CSV before server_wrap applies auth
        // (routes:83-95); keep that order for malformed uploads.
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                                      core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Result = ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult;
        std::optional<Result> parsed{};
        std::string parse_error{};
        if (is_json_body(body)) {
            auto decoded = serde::Ser::deserialize<Result>("application/json", body);
            if (!decoded) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                                          core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            parsed = std::move(*decoded);
        } else {
            // validate_and_get_records (api_models:340-379) aborts the whole upload on any
            // row parse failure and reports at most ten row messages.
            parsed = parse_retry_stats_csv(unwrap_multipart_file(body), parse_error);
            if (!parsed) {
                send_bad_request(res, send,
                                 "failed to parse the retry stats migration CSV file: " + parse_error);
                return;
            }
        }

        if (const auto auth_error = authorize(req, "revenue_recovery_retry_stats_migration")) {
            send_error(res, send, *auth_error);
            return;
        }

        using Error = ::app::dto::revenue_recovery_data_backfill::CsvParsingError;
        const auto row_error = [](std::size_t row, std::string message) {
            Error error{};
            error.set_row_number(row);
            error.set_error(std::move(message));
            return error;
        };

        // migration.rs:25-60: phase 1 validates EVERY row before any write, then rejects
        // duplicate cluster keys. Nothing is persisted while validation errors exist.
        std::vector<std::pair<std::size_t, ValidatedRow>> validated_rows{};
        std::vector<Error> validation_errors{};
        std::unordered_set<std::string> seen_keys{};
        for (const auto& [row_number, record] : parsed->get_records()) {
            auto key = validate_cluster_key(record.get_cluster_key());
            if (!key) {
                validation_errors.push_back(row_error(
                    row_number, "invalid cluster_key '" + record.get_cluster_key() + "'"));
                continue;
            }
            std::string doc_error{};
            auto document = parse_stats_document(record.get_stats(), doc_error);
            if (!document) {
                validation_errors.push_back(row_error(
                    row_number, "stats is not a valid stats document: " + doc_error));
                continue;
            }
            if (!seen_keys.insert(*key).second) {
                validation_errors.push_back(row_error(row_number, "duplicate cluster_key present"));
                continue;
            }
            validated_rows.emplace_back(row_number,
                                        ValidatedRow{*key, record.get_stats(), std::move(*document)});
        }

        std::vector<Error> row_errors{};
        std::size_t processed_records{0};
        if (validation_errors.empty()) {
            // migration.rs:75-99: phase 2 persists sequentially; every write is a
            // whole-document replace, so re-running the file is safe.
            for (const auto& [row_number, row] : validated_rows) {
                const auto replaced = replace_retry_stats_document(row.key, row.stats);
                if (!replaced || !*replaced) {
                    row_errors.push_back(row_error(
                        row_number,
                        "per-key redis lock stayed contended; row skipped, retry the file"));
                }
            }
            processed_records = validated_rows.size() - row_errors.size();
        } else {
            row_errors = std::move(validation_errors);
        }

        record_flow("RecoveryRetryStatsMigration", "revenue_recovery_retry_stats_migration",
                    processed_records, row_errors.size());

        using Response = ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationResponse;
        Response response;
        response.set_processed_records(processed_records);
        response.set_failed_records(row_errors.size());
        response.set_row_errors(std::move(row_errors));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:106-128 ->
     * core/revenue_recovery_data_backfill.rs:157-261.
     */
    void handle_update_revenue_recovery_additional_redis_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                authorize(req, "update_revenue_recovery_additional_redis_data")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                                      core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Upstream ScheduledAtUpdate has a custom Deserializer (api_models:228-253):
        // a JSON string "null" -> SetToNull, any other string -> iso8601 datetime.
        // The local DTO enum cannot carry a datetime, so handle that field from the
        // generic tree first and strip it before the typed decode.
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                                      core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        std::optional<ScheduledUpdate> scheduled_update{};
        if (auto* root = std::get_if<serde::Value::Object>(&generic->get())) {
            const auto found = root->get(std::string{"scheduled_at"});
            if (found) {
                if (const auto* text = std::get_if<std::string>(&found->get())) {
                    std::string lowered{};
                    lowered.reserve(text->size());
                    for (const char ch : *text) {
                        lowered.push_back(static_cast<char>(std::tolower(
                            static_cast<unsigned char>(ch))));
                    }
                    if (lowered == "null") {
                        scheduled_update = ScheduledUpdate{ScheduledKind::Null, std::string{}};
                    } else if (const auto dt = detail::PrimitiveDateTime::parse_iso8601(*text)) {
                        scheduled_update = ScheduledUpdate{ScheduledKind::DateTime,
                                                           dt->storage_string()};
                    } else {
                        send_bad_request(res, send, "Invalid scheduled_at: expected null or "
                                                    "datetime iso8601");
                        return;
                    }
                } else {
                    send_bad_request(
                        res, send,
                        "Expected null variable or datetime iso8601 for scheduled_at");
                    return;
                }
            }
        }

        using Input = ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest;
        auto parsed = serde::Ser::from_value<Input>(*generic);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                                      core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Secret<String, PhoneNumberStrategy> Debug is masked; local SecretString::masked()
        // is the in-process substitute for that presentation.
        const auto secret = ::app::crypto::SecretString{parsed->get_payment_processor_token()};
        const auto& token = parsed->get_payment_processor_token();
        const auto& connector_customer_id = parsed->get_connector_customer_id();

        detail::LocalRedis redis{m_context->cache()};
        auto tokens = redis.get_hash_fields(detail::LocalRedis::tokens_key(connector_customer_id));
        const auto found = tokens.find(token);
        if (found == tokens.end()) {
            // core:173-180 -> GenericNotFoundError (IR_37, 404).
            send_api_error(res, send, interfaces::io::types::Status::NOT_FOUND, "invalid_request",
                           "IR_37",
                           "Token '" + secret.masked()
                               + "' not found for connector customer id:- '"
                               + connector_customer_id + "'");
            return;
        }

        auto status = decode_token_status(found->second);
        if (!status) {
            send_api_error(res, send, interfaces::io::types::Status::NOT_FOUND, "invalid_request",
                           "IR_37",
                           "Token '" + secret.masked()
                               + "' not found for connector customer id:- '"
                               + connector_customer_id + "'");
            return;
        }

        std::vector<std::string> updated_fields{};
        if (scheduled_update) {
            if (scheduled_update->kind == ScheduledKind::Null) {
                status->scheduled_at = std::nullopt; // core:196-204
                updated_fields.emplace_back("scheduled_at: set to null");
            } else {
                status->scheduled_at = scheduled_update->datetime; // core:185-195
                updated_fields.push_back("scheduled_at: " + scheduled_update->datetime);
            }
        }
        if (parsed->get_is_hard_decline()) {
            status->is_hard_decline = *parsed->get_is_hard_decline(); // core:211-215
            updated_fields.push_back(std::string{"is_hard_decline: "}
                                     + (*parsed->get_is_hard_decline() ? "true" : "false"));
        }
        if (parsed->get_error_code()) {
            status->error_code = *parsed->get_error_code(); // core:217-221
            updated_fields.push_back("error_code: " + *parsed->get_error_code());
        }
        status->modified_at = detail::PrimitiveDateTime::now().storage_string();
        tokens[token] = encode_token_status(*status);

        // core:224-238: update_or_add... with the configured Redis TTL.
        redis.set_hash_fields(detail::LocalRedis::tokens_key(connector_customer_id), tokens,
                              redis_ttl_seconds());

        // Local DB shadow keeps the repository row in sync with the Redis substitute.
        auto repository = m_context->repository();
        if (auto existing = repository.find_revenue_recovery_token_by_string(token)) {
            auto payload = update_payload_fields(existing->get_payload(), &*parsed,
                                                 scheduled_update.has_value()
                                                     ? std::optional<ScheduledUpdate>{*scheduled_update}
                                                     : std::nullopt);
            existing->set_payload(std::move(payload));
            (void)repository.update_revenue_recovery_token(*existing);
        }

        const std::string updated_fields_str = updated_fields.empty()
                                                   ? std::string{"no fields were updated"}
                                                   : join(updated_fields, ", ");
        record_flow("RecoveryDataBackfill",
                    "revenue_recovery_update_additional_redis_data", std::size_t{1},
                    std::size_t{0});

        using Response = ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse;
        Response response;
        response.set_updated(true);
        response.set_message("Successfully updated token '" + secret.masked()
                             + "' for connector customer '" + connector_customer_id
                             + "'. Updated fields: " + updated_fields_str);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:130-160 ->
     * core/revenue_recovery_data_backfill.rs:66-117 ->
     * RedisTokenManager::unlock_connector_customer_status
     * (types/storage/revenue_recovery_redis_operation.rs:232-286).
     */
    void handle_revenue_recovery_data_backfill_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize(req, "revenue_recovery_data_backfill_status")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto segments = ::app::http::RequestPath{}(req);
        using Request = ::app::dto::revenue_recovery_data_backfill::UnlockStatusRequest;
        Request parsed;
        if (segments.count() < 2) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                                      core::error::AppErrorKind::INVALID_REQUEST,
                                      "missing connector customer id or payment intent id in path"));
            return;
        }
        parsed.set_connector_customer_id(std::string{segments.at(segments.count() - 2).value_or("")});
        ::app::dto::revenue_recovery_data_backfill::GlobalPaymentIdValue payment_id;
        payment_id.set_value(std::string{segments.at(segments.count() - 1).value_or("")});
        parsed.set_payment_intent_id(std::move(payment_id));

        const auto& customer_id = parsed.get_connector_customer_id();
        const auto& expected_owner = parsed.get_payment_intent_id().get_value();

        detail::LocalRedis redis{m_context->cache()};
        const auto native_key = detail::LocalRedis::status_key(customer_id);
        const auto legacy_lock_key = status_lock_key(customer_id);
        auto stored = redis.get_key(native_key);
        bool native_exists = stored.has_value();
        if (!native_exists) {
            // Keep the LockRegistry contract used by the sibling redis handler:
            // lock:revenue_recovery.status:{id} holds the payment id as a plain string.
            const auto legacy = m_context->cache().get(lock_scoped_key(legacy_lock_key));
            if (legacy.hit && !core::cache::IsMiss{}(legacy.value)) {
                stored = core::cache::value_as<std::string>(legacy.value, std::string{});
                native_exists = true;
            }
        }

        if (!native_exists) {
            // Redis GET on a missing key is an error (RedisConnectionError) ->
            // core:89-93 InternalServerError.
            send_api_error(res, send, interfaces::io::types::Status::INTERNAL_SERVER_ERROR,
                           "server_not_available", "HE_00", "Something went wrong");
            return;
        }
        if (!stored || *stored != expected_owner) {
            // RedisError::DeleteFailed -> InvalidPaymentIdProvided (core:81-88).
            const auto resource = "The Status token for connector costumer id:- " + customer_id
                                  + " is locked by different PaymentIntent ID";
            send_api_error(res, send, interfaces::io::types::Status::NOT_FOUND, "invalid_request",
                           "IR_48",
                           "The Status token for connector costumer id " + resource
                               + " is locked by different PaymentIntent ID");
            return;
        }
        redis.delete_key(native_key);
        (void)m_context->locks().release(legacy_lock_key);

        record_flow("RecoveryDataBackfillStatus", "revenue_recovery_data_backfill_status",
                    std::size_t{1}, std::size_t{0});

        using Response = ::app::dto::revenue_recovery_data_backfill::UnlockStatusResponse;
        Response response;
        response.set_unlocked(true);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "v2/recovery/data-backfill"}
            .add_route(
                Route{""}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_revenue_recovery_data_backfill(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_data_backfill")
            )
            .add_router(Router{router, "status"}.add_router(Router{router, ":connector_cutomer_id"}.add_route(
                Route{":payment_intent_id"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_revenue_recovery_data_backfill_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_data_backfill_status")
            )))
            .add_route(
                Route{"update-token"}
                    .put([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                std::function<void()> send) {
                        handle_update_revenue_recovery_additional_redis_data(req, res,
                                                                             std::move(send));
                    })
                    .summary("Hyperswitch update_revenue_recovery_additional_redis_data")
            )
            .add_route(
                Route{"retry-stats"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res,
                                 std::function<void()> send) {
                        handle_revenue_recovery_retry_stats_migration(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_retry_stats_migration")
            );
    }

private:
    using BackfillRequest = ::app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest;
    using CardData = ::app::dto::revenue_recovery_data_backfill::ComprehensiveCardData;
    using UpdateRequest = ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest;

    enum class ScheduledKind { Null, DateTime };
    struct ScheduledUpdate
    {
        ScheduledKind kind{ScheduledKind::Null};
        std::string datetime{};
    };
    struct ValidatedRow
    {
        std::string key{};
        std::string stats{};
        detail::StatsDocument document{};
    };

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req, std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::string status_lock_key(std::string_view customer_id) const
    {
        return "revenue_recovery.status:" + std::string{customer_id};
    }

    [[nodiscard]] std::string lock_scoped_key(std::string_view key) const
    {
        return "lock:" + std::string{key};
    }

    [[nodiscard]] std::int64_t lock_ttl_seconds() const noexcept
    {
        return 3600;
    }

    [[nodiscard]] std::int64_t redis_ttl_seconds() const noexcept
    {
        return 3600;
    }

    void record_flow(std::string_view flow, std::string_view counter, std::size_t processed,
                     std::size_t failed) const
    {
        m_context->metrics().record_flow(flow);
        m_context->metrics().counter(std::string{counter} + ".requests").increment();
        m_context->metrics().counter(std::string{counter} + ".processed_records")
            .increment(processed);
        m_context->metrics().counter(std::string{counter} + ".failed_records").increment(failed);
    }

    // ── core/revenue_recovery_data_backfill.rs:23-64 / :263-333 ───────────────

    [[nodiscard]] bool process_payment_method_record(
        const BackfillRequest& record,
        const std::optional<detail::PrimitiveDateTime>& cutoff_datetime
    ) const
    {
        // core:269-289: build errors fall back to card-only data and still process.
        const auto card_data = build_comprehensive_card_data(record);
        const auto card_json = serde::Ser::encode_json(card_data);

        const auto& token = record.get_token();
        if (token && !token->empty()) {
            // core:301-319: update_redis_token_with_comprehensive_card_data; a missing
            // token is a failed record (BackfillError::RedisError).
            if (!update_redis_token_with_comprehensive_card_data(
                    record.get_customer_id_resp(), *token, card_data, cutoff_datetime)) {
                return false;
            }
            // Local DB shadow of the Redis token record (durability for the substitute).
            persist_token(record, card_json, cutoff_datetime);
            // Stand-in for the status lock created by the upstream recovery flow; also
            // preserves the key contract consumed by revenue_recovery_redis.cppm.
            if (const auto& payment_id = record.get_connector_payment_id();
                payment_id && !payment_id->empty()) {
                (void)m_context->locks().acquire(
                    status_lock_key(record.get_customer_id_resp()), lock_ttl_seconds(), *payment_id);
            }
        }
        return true;
    }

    /** Port of RedisTokenManager::update_redis_token_with_comprehensive_card_data
     *  (revenue_recovery_redis_operation.rs:1148-1261). */
    [[nodiscard]] bool update_redis_token_with_comprehensive_card_data(
        const std::string& customer_id,
        const std::string& token,
        const CardData& card_data,
        const std::optional<detail::PrimitiveDateTime>& cutoff_datetime
    ) const
    {
        detail::LocalRedis redis{m_context->cache()};
        const auto key = detail::LocalRedis::tokens_key(customer_id);
        auto tokens = redis.get_hash_fields(key);
        auto it = tokens.find(token);
        if (it == tokens.end()) {
            // Bridge: seed the local Redis hash from the DB shadow so a token
            // persisted by the earlier C++ path behaves like an existing Redis token.
            auto repository = m_context->repository();
            auto shadow = repository.find_revenue_recovery_token_by_string(token);
            if (!shadow || shadow->get_merchant_id() != customer_id) {
                return false; // core:1162-1170 ValueNotFound -> failed record.
            }
            detail::PaymentProcessorTokenStatus seeded{};
            seeded.payment_processor_token_details.payment_processor_token = token;
            tokens.emplace(token, encode_token_status(seeded));
            it = tokens.find(token);
        }
        auto status = decode_token_status(it->second);
        if (!status) {
            return false;
        }

        // core:1172-1192: only overwrite fields that are present.
        if (card_data.get_card_type()) {
            status->payment_processor_token_details.card_type = *card_data.get_card_type();
        }
        if (card_data.get_card_exp_month()) {
            status->payment_processor_token_details.expiry_month = *card_data.get_card_exp_month();
        }
        if (card_data.get_card_exp_year()) {
            status->payment_processor_token_details.expiry_year = *card_data.get_card_exp_year();
        }
        if (card_data.get_card_network()) {
            status->payment_processor_token_details.card_network =
                card_data.get_card_network()->get_value();
        }
        if (card_data.get_card_issuer()) {
            status->payment_processor_token_details.card_issuer = *card_data.get_card_issuer();
        }
        // core:1193-1197: a complete replacement when history was provided.
        if (card_data.get_daily_retry_history()) {
            status->daily_retry_history = *card_data.get_daily_retry_history();
        }
        // core:1199-1219: clear scheduled_at strictly older than the cutoff.
        if (status->scheduled_at) {
            const auto scheduled = detail::PrimitiveDateTime::parse(*status->scheduled_at);
            if (cutoff_datetime && scheduled && *scheduled < *cutoff_datetime) {
                status->scheduled_at = std::nullopt;
            }
        }
        status->modified_at = detail::PrimitiveDateTime::now().storage_string(); // core:1221
        // core:1223-1240: extend an existing history, never create one.
        if (card_data.get_account_update_history() && status->account_update_history) {
            const auto& incoming = *card_data.get_account_update_history();
            auto& existing = *status->account_update_history;
            existing.insert(existing.end(), incoming.begin(), incoming.end());
        }
        if (card_data.get_is_active()) {
            status->is_active = *card_data.get_is_active(); // core:1242-1245
        }

        it->second = encode_token_status(*status);
        // core:1247-1253: save the WHOLE hash back with the Redis TTL.
        redis.set_hash_fields(key, tokens, redis_ttl_seconds());
        return true;
    }

    // ── core/revenue_recovery_data_backfill.rs:390-523 ────────────────────────

    [[nodiscard]] CardData build_comprehensive_card_data(const BackfillRequest& record) const
    {
        CardData card_data{};
        if (const auto& payment_method = record.get_payment_method_sub_type()) {
            const auto& value = payment_method->get_value();
            std::string lowered{};
            lowered.reserve(value.size());
            for (const char ch : value) {
                lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            }
            if (lowered == "credit") {
                card_data.set_card_type(std::string{"credit"});
            } else if (lowered == "debit") {
                card_data.set_card_type(std::string{"debit"});
            } else if (lowered == "card") {
                card_data.set_card_type(std::string{"card"});
            } else {
                card_data.set_card_type(std::string{"card"}); // default for all other subtypes
            }
        }

        std::string month{};
        std::string year{};
        if (!parse_expiration_date(record.get_exp_date(), month, year)) {
            // build_comprehensive_card_data error -> process_payment_method_record fallback.
            CardData fallback{};
            fallback.set_card_type(std::string{"card"});
            return fallback;
        }
        if (!month.empty() && !year.empty()) {
            card_data.set_card_exp_month(month);
            card_data.set_card_exp_year(year);
        }

        card_data.set_card_network(record.get_card_network());
        if (const auto& issuer = record.get_clean_bank_name(); issuer && !issuer->empty()) {
            card_data.set_card_issuer(*issuer);
        }
        if (const auto& country = record.get_country_name(); country && !country->empty()) {
            card_data.set_card_issuing_country(*country);
        }
        card_data.set_is_active(record.get_is_active());
        card_data.set_account_update_history(record.get_account_update_history());
        card_data.set_daily_retry_history(parse_daily_retry_history(record.get_daily_retry_history()));
        return card_data;
    }

    [[nodiscard]] bool parse_expiration_date(
        const std::optional<std::string>& raw, std::string& month, std::string& year
    ) const
    {
        month.clear();
        year.clear();
        if (!raw || raw->empty()) {
            return true; // core:519-522 -> (None, None).
        }
        const auto slash = raw->find('/');
        if (slash == std::string::npos) {
            return false;
        }
        const auto trim = [](std::string_view part) {
            const auto first = part.find_first_not_of(" \t");
            if (first == std::string_view::npos) {
                return std::string{};
            }
            const auto last = part.find_last_not_of(" \t");
            return std::string{part.substr(first, last - first + 1)};
        };
        const auto parsed_month = trim(std::string_view{*raw}.substr(0, slash));
        const auto parsed_year = trim(std::string_view{*raw}.substr(slash + 1));
        if (parsed_month.empty() || parsed_year.empty()) {
            return false;
        }
        // u8::parse is strict digits: no signs, no trailing characters.
        for (const char ch : parsed_month) {
            if (ch < '0' || ch > '9') {
                return false;
            }
        }
        const auto month_number = detail::PrimitiveDateTime::read_uint(parsed_month, 0,
                                                                       parsed_month.size());
        if (!month_number || *month_number < std::uint32_t{1} || *month_number > std::uint32_t{12}) {
            return false;
        }
        if (parsed_year.size() == std::size_t{4}) {
            for (const char ch : parsed_year) {
                if (ch < '0' || ch > '9') {
                    return false;
                }
            }
            year = parsed_year.substr(2);
        } else if (parsed_year.size() == std::size_t{2}) {
            for (const char ch : parsed_year) {
                if (ch < '0' || ch > '9') {
                    return false;
                }
            }
            year = parsed_year;
        } else {
            return false;
        }
        month = parsed_month;
        return true;
    }

    /** Port of parse_daily_retry_history (core:335-388). */
    [[nodiscard]] std::optional<std::unordered_map<std::string, std::int32_t>>
    parse_daily_retry_history(const std::optional<std::string>& raw) const
    {
        if (!raw || raw->empty()) {
            return std::nullopt;
        }
        auto doc = serde::Ser::decode_generic("application/json", *raw);
        if (!doc) {
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&doc->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        std::unordered_map<std::string, std::int32_t> history{};
        for (const auto& [key, value] : *object) {
            const auto parsed = detail::PrimitiveDateTime::parse(key);
            if (!parsed) {
                continue; // core:364-367 logs and skips unparseable keys.
            }
            std::int32_t count{0};
            if (const auto* number = std::get_if<std::int64_t>(&value.get())) {
                count = static_cast<std::int32_t>(*number);
            } else if (const auto* number = std::get_if<double>(&value.get())) {
                count = static_cast<std::int32_t>(*number);
            } else {
                continue;
            }
            history.insert_or_assign(parsed->storage_string(), count);
        }
        return history;
    }

    // ── retry-stats migration (core/revenue_recovery/retry_stats/migration.rs) ─

    [[nodiscard]] std::optional<std::string> validate_cluster_key(std::string_view raw) const
    {
        // RetryStatsClusterKey::from_db_string (retry_stats_cluster_key.rs:195-229):
        // "v1|<error_code>/<card_type>/<issuer>", exactly three segments, and valid
        // percent escaping within each segment.
        const auto separator = raw.find('|');
        if (separator == std::string_view::npos || raw.substr(0, separator) != "v1") {
            return std::nullopt;
        }
        const auto rest = raw.substr(separator + 1);
        std::vector<std::string_view> segments{};
        std::size_t start{0};
        while (true) {
            const auto pos = rest.find('/', start);
            segments.push_back(rest.substr(start, pos == std::string_view::npos
                                                     ? std::string_view::npos
                                                     : pos - start));
            if (pos == std::string_view::npos) {
                break;
            }
            start = pos + 1;
        }
        if (segments.size() != std::size_t{3}) {
            return std::nullopt;
        }
        for (const auto& segment : segments) {
            for (std::size_t i = 0; i < segment.size(); ++i) {
                if (segment[i] != '%') {
                    continue;
                }
                if (i + std::size_t{2} >= segment.size()
                    || !std::isxdigit(static_cast<unsigned char>(segment[i + 1]))
                    || !std::isxdigit(static_cast<unsigned char>(segment[i + 2]))) {
                    return std::nullopt;
                }
                i += 2;
            }
        }
        return std::string{raw};
    }

    [[nodiscard]] std::optional<detail::StatsDocument>
    parse_stats_document(std::string_view raw, std::string& error) const
    {
        auto generic = serde::Ser::decode_generic("application/json", raw);
        if (!generic) {
            error = "expected an object with dow/dom/hod arrays";
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&generic->get());
        if (object == nullptr) {
            error = "expected an object with dow/dom/hod arrays";
            return std::nullopt;
        }
        detail::StatsDocument document{};
        for (const auto* name : {"dow", "dom", "hod"}) {
            const auto found = object->get(std::string{name});
            std::vector<detail::SlotCounter>* target = nullptr;
            if (std::string_view{name} == "dow") {
                target = &document.dow;
            } else if (std::string_view{name} == "dom") {
                target = &document.dom;
            } else {
                target = &document.hod;
            }
            const std::size_t expected_slots =
                std::string_view{name} == "dow" ? std::size_t{7}
                : std::string_view{name} == "dom" ? std::size_t{31}
                                                  : std::size_t{24};
            if (!found) {
                // #[serde(default)] -> all-zero array of the fixed length.
                target->resize(expected_slots, detail::SlotCounter{});
                continue;
            }
            const auto* array = std::get_if<serde::Value::Array>(&found->get());
            if (array == nullptr) {
                error = std::string{name} + " must be an array";
                return std::nullopt;
            }
            if (array->size() != expected_slots) {
                error = std::string{name} + " has the wrong number of slots";
                return std::nullopt;
            }
            for (const auto& element : *array) {
                const auto* slot = std::get_if<serde::Value::Object>(&element.get());
                if (slot == nullptr) {
                    error = std::string{name} + " contains a non-object slot";
                    return std::nullopt;
                }
                detail::SlotCounter counter{};
                const auto read = [&](std::string_view field, std::uint64_t& out) {
                    const auto field_found = slot->get(std::string{field});
                    if (!field_found) {
                        return true; // serde(default) -> 0.
                    }
                    if (const auto* number = std::get_if<std::int64_t>(&field_found->get())) {
                        if (*number < 0) {
                            return false; // serde cannot decode a negative u64.
                        }
                        out = static_cast<std::uint64_t>(*number);
                        return true;
                    }
                    return false;
                };
                if (!read("n", counter.n) || !read("k", counter.k)) {
                    error = std::string{name} + " has a non-numeric slot counter";
                    return std::nullopt;
                }
                target->push_back(counter);
            }
        }

        // validate_invariants (retry_stats_document.rs:68-97).
        const auto fill = [](std::vector<detail::SlotCounter>& family, std::size_t expected) {
            family.resize(expected, detail::SlotCounter{});
        };
        fill(document.dow, std::size_t{7});
        fill(document.dom, std::size_t{31});
        fill(document.hod, std::size_t{24});
        const auto totals = [](const std::vector<detail::SlotCounter>& family, bool successes) {
            std::uint64_t sum{0};
            for (const auto& slot : family) {
                sum += successes ? slot.k : slot.n;
            }
            return sum;
        };
        for (const auto* family : {&document.dow, &document.dom, &document.hod}) {
            for (const auto& slot : *family) {
                if (slot.k > slot.n) {
                    error = "slot counter has k>n";
                    return std::nullopt;
                }
            }
        }
        const auto [dow_n, dom_n, hod_n] =
            std::tuple{totals(document.dow, false), totals(document.dom, false),
                       totals(document.hod, false)};
        if (dow_n != dom_n || dom_n != hod_n) {
            error = std::format("n totals differ across slot families (dow={}, dom={}, hod={})",
                                dow_n, dom_n, hod_n);
            return std::nullopt;
        }
        const auto [dow_k, dom_k, hod_k] =
            std::tuple{totals(document.dow, true), totals(document.dom, true),
                       totals(document.hod, true)};
        if (dow_k != dom_k || dom_k != hod_k) {
            error = std::format("k totals differ across slot families (dow={}, dom={}, hod={})",
                                dow_k, dom_k, hod_k);
            return std::nullopt;
        }
        return document;
    }

    /** Port of record::replace_retry_stats_document (retry_stats/record.rs:79-108, 112-142). */
    [[nodiscard]] std::optional<bool>
    replace_retry_stats_document(const std::string& cluster_key, std::string_view stats) const
    {
        detail::LocalRedis redis{m_context->cache()};
        const auto lock_key = "revenue_recovery_retry_stats_lock:" + cluster_key;
        const std::int64_t ttl_seconds{30};
        bool acquired{false};
        for (int attempt = 0; attempt < 3; ++attempt) {
            if (redis.set_key_if_not_exists_with_expiry(lock_key, "migration", ttl_seconds)) {
                acquired = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
        if (!acquired) {
            return std::false_type::value; // Ok(false): contended, nothing written.
        }
        const bool existed = m_context->config_store().get("revenue_recovery_retry_stats",
                                                           cluster_key).has_value();
        m_context->config_store().put("revenue_recovery_retry_stats", cluster_key,
                                      std::string{stats}); // whole-document replace.
        // Release only while still the owner (record.rs:270-284).
        if (redis.get_key(lock_key) == std::optional<std::string>{"migration"}) {
            redis.delete_key(lock_key);
        }
        return !existed; // Ok(true) - upstream ignores insert/update distinction here.
    }

    // ── token status JSON codec + local DB shadow ─────────────────────────────

    [[nodiscard]] static std::string encode_token_status(const detail::PaymentProcessorTokenStatus& s)
    {
        return serde::Ser::encode_json(s);
    }

    /**
     * Decodes a stored token status like upstream serde: unknown fields are ignored and
     * every optional field may be absent. Generic navigation keeps the local Redis
     * substitute tolerant of partially populated token records.
     */
    [[nodiscard]] static std::optional<detail::PaymentProcessorTokenStatus>
    decode_token_status(std::string_view text)
    {
        auto doc = serde::Ser::decode_generic("application/json", text);
        if (!doc) {
            return std::nullopt;
        }
        const auto* root = std::get_if<serde::Value::Object>(&doc->get());
        if (root == nullptr) {
            return std::nullopt;
        }
        const auto read_string = [&](const serde::Value::Object& object, std::string_view key)
            -> std::optional<std::string> {
            const auto found = object.get(std::string{key});
            if (!found) { return std::nullopt; }
            if (const auto* text = std::get_if<std::string>(&found->get())) {
                return *text;
            }
            return std::nullopt;
        };
        const auto read_bool = [&](const serde::Value::Object& object, std::string_view key)
            -> std::optional<bool> {
            const auto found = object.get(std::string{key});
            if (!found) { return std::nullopt; }
            if (const auto* value = std::get_if<bool>(&found->get())) {
                return *value;
            }
            return std::nullopt;
        };

        detail::PaymentProcessorTokenStatus status{};
        const auto details_found = root->get(std::string{"payment_processor_token_details"});
        if (details_found) {
            if (const auto* details = std::get_if<serde::Value::Object>(&details_found->get())) {
                auto& out = status.payment_processor_token_details;
                if (auto v = read_string(*details, "payment_processor_token")) {
                    out.payment_processor_token = *v;
                }
                out.expiry_month = read_string(*details, "expiry_month");
                out.expiry_year = read_string(*details, "expiry_year");
                out.card_issuer = read_string(*details, "card_issuer");
                out.last_four_digits = read_string(*details, "last_four_digits");
                out.card_network = read_string(*details, "card_network");
                out.card_type = read_string(*details, "card_type");
                out.card_isin = read_string(*details, "card_isin");
            }
        }
        if (auto v = read_string(*root, "inserted_by_attempt_id")) {
            status.inserted_by_attempt_id = *v;
        }
        status.error_code = read_string(*root, "error_code");
        status.scheduled_at = read_string(*root, "scheduled_at");
        status.is_hard_decline = read_bool(*root, "is_hard_decline");
        status.modified_at = read_string(*root, "modified_at");
        status.is_active = read_bool(*root, "is_active");
        if (const auto history_found = root->get(std::string{"daily_retry_history"})) {
            if (const auto* history = std::get_if<serde::Value::Object>(&history_found->get())) {
                for (const auto& [key, value] : *history) {
                    if (const auto* number = std::get_if<std::int64_t>(&value.get())) {
                        status.daily_retry_history[key] = static_cast<std::int32_t>(*number);
                    } else if (const auto* number = std::get_if<double>(&value.get())) {
                        status.daily_retry_history[key] = static_cast<std::int32_t>(*number);
                    }
                }
            }
        }
        if (const auto history_found = root->get(std::string{"account_update_history"})) {
            auto history = serde::Ser::from_value<
                std::vector<app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord>>(
                *history_found);
            if (history) {
                status.account_update_history = std::move(*history);
            }
        }
        if (const auto threshold_found = root->get(std::string{"decision_threshold"})) {
            if (const auto* number = std::get_if<double>(&threshold_found->get())) {
                status.decision_threshold = *number;
            }
        }
        return status;
    }

    [[nodiscard]] std::string
    update_payload_fields(std::string_view payload, const UpdateRequest* request,
                          const std::optional<ScheduledUpdate>& scheduled) const
    {
        std::string out{payload.empty() ? std::string{"{}"} : std::string{payload}};
        auto doc = serde::Ser::decode_generic("application/json", out);
        if (!doc) {
            return out;
        }
        auto* object = std::get_if<serde::Value::Object>(&doc->get());
        if (object == nullptr) {
            return out;
        }
        if (scheduled) {
            if (scheduled->kind == ScheduledKind::Null) {
                (*object)["scheduled_at"] = serde::Value{};
            } else {
                (*object)["scheduled_at"] = serde::Value{scheduled->datetime};
            }
        }
        if (request && request->get_is_hard_decline()) {
            (*object)["is_hard_decline"] = serde::Value{*request->get_is_hard_decline()};
        }
        if (request && request->get_error_code()) {
            (*object)["error_code"] = serde::Value{*request->get_error_code()};
        }
        return serde::Ser::encode_json(*doc);
    }

    void persist_token(const BackfillRequest& record, const std::string& card_json,
                       const std::optional<detail::PrimitiveDateTime>& cutoff_datetime) const
    {
        using TokenRecord = ::app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord;
        auto repository = m_context->repository();
        const auto& token = record.get_token();

        TokenRecord token_record{};
        if (token && !token->empty()) {
            token_record = repository.find_revenue_recovery_token_by_string(*token)
                               .value_or(TokenRecord{});
        }
        token_record.set_token(token.value_or(""));
        token_record.set_merchant_id(record.get_customer_id_resp());
        token_record.set_payment_id(record.get_connector_payment_id().value_or(""));
        token_record.set_status("active");
        token_record.set_payload(card_json);
        token_record.set_expires_at(cutoff_datetime ? cutoff_datetime->storage_string()
                                                    : std::string{});
        if (token_record.get_created_at().empty()) {
            token_record.set_created_at(
                std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count()));
        }
        if (token_record.get_id() == 0) {
            (void)repository.save_revenue_recovery_token(token_record);
        } else {
            (void)repository.update_revenue_recovery_token(token_record);
        }
    }

    // ── in-process multipart/CSV substitute ───────────────────────────────────

    [[nodiscard]] static bool is_json_body(std::string_view body)
    {
        const auto first = body.find_first_not_of(" \t\r\n");
        return first != std::string_view::npos
               && (body[first] == '{' || body[first] == '[');
    }

    /** Extract the `file` part of a multipart body; pass raw CSV through unchanged. */
    [[nodiscard]] static std::string unwrap_multipart_file(std::string_view body)
    {
        const auto name_pos = body.find("name=\"file\"");
        if (name_pos == std::string_view::npos) {
            return std::string{body};
        }
        const auto header_end = body.find("\r\n\r\n", name_pos);
        if (header_end == std::string_view::npos) {
            return std::string{body};
        }
        const auto start = header_end + std::size_t{4};
        const auto end = body.find("\r\n--", start);
        std::string out{body.substr(start, end == std::string_view::npos
                                              ? std::string_view::npos
                                              : end - start)};
        while (!out.empty() && (out.back() == '\r' || out.back() == '\n')) {
            out.pop_back();
        }
        return out;
    }

    [[nodiscard]] static std::vector<std::string> split_csv_line(std::string_view line)
    {
        std::vector<std::string> out{};
        std::string current{};
        bool quoted{false};
        for (std::size_t i = 0; i < line.size(); ++i) {
            const char ch = line[i];
            if (quoted) {
                if (ch == '"') {
                    if (i + std::size_t{1} < line.size() && line[i + 1] == '"') {
                        current.push_back('"');
                        ++i;
                    } else {
                        quoted = false;
                    }
                } else {
                    current.push_back(ch);
                }
            } else if (ch == '"') {
                quoted = true;
            } else if (ch == ',') {
                out.push_back(current);
                current.clear();
            } else {
                current.push_back(ch);
            }
        }
        out.push_back(current);
        return out;
    }

    [[nodiscard]] static std::vector<std::pair<std::size_t, std::string>> csv_rows(
        std::string_view csv
    )
    {
        std::vector<std::pair<std::size_t, std::string>> rows{};
        std::size_t row_number{1};
        std::size_t start{0};
        while (start <= csv.size()) {
            const auto newline = csv.find('\n', start);
            std::string_view line = csv.substr(
                start, newline == std::string_view::npos ? std::string_view::npos
                                                         : newline - start);
            if (!line.empty() && line.back() == '\r') {
                line.remove_suffix(1);
            }
            if (!line.empty()) {
                rows.emplace_back(row_number, std::string{line});
            }
            ++row_number;
            if (newline == std::string_view::npos) {
                break;
            }
            start = newline + 1;
        }
        return rows;
    }

    [[nodiscard]] static std::optional<std::string>
    cell(const std::vector<std::string>& headers, const std::vector<std::string>& cells,
         std::string_view name)
    {
        for (std::size_t i = 0; i < headers.size(); ++i) {
            if (headers[i] == name && i < cells.size() && !cells[i].empty()) {
                return cells[i];
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<::app::dto::revenue_recovery_data_backfill::CsvParsingResult>
    parse_backfill_csv(std::string_view csv, std::string& error) const
    {
        using namespace ::app::dto::revenue_recovery_data_backfill;
        CsvParsingResult result{};
        const auto rows = csv_rows(csv);
        if (rows.empty()) {
            // csv::Reader on an empty file yields no rows -> Ok(empty CsvParsingResult).
            return result;
        }
        auto headers = split_csv_line(rows.front().second);
        if (!headers.empty() && headers.front().starts_with("\xEF\xBB\xBF")) {
            headers.front().erase(0, 3);
        }
        std::vector<RevenueRecoveryBackfillRequest> records{};
        std::vector<CsvParsingError> failed{};
        for (std::size_t index = 1; index < rows.size(); ++index) {
            const auto cells = split_csv_line(rows[index].second);
            if (cells.size() != headers.size()) {
                CsvParsingError row_error{};
                row_error.set_row_number(index + 1);
                row_error.set_error("found record with " + std::to_string(cells.size())
                                    + " fields but header has " + std::to_string(headers.size()));
                failed.push_back(std::move(row_error));
                continue;
            }
            RevenueRecoveryBackfillRequest record{};
            const auto customer = cell(headers, cells, "customer_id_resp");
            if (!customer) {
                CsvParsingError row_error{};
                row_error.set_row_number(index + 1);
                row_error.set_error("missing field customer_id_resp");
                failed.push_back(std::move(row_error));
                continue;
            }
            record.set_customer_id_resp(*customer);
            record.set_bin_number(cell(headers, cells, "bin_number"));
            record.set_connector_payment_id(cell(headers, cells, "connector_payment_id"));
            record.set_token(cell(headers, cells, "token"));
            record.set_exp_date(cell(headers, cells, "exp_date"));
            record.set_clean_bank_name(cell(headers, cells, "clean_bank_name"));
            record.set_country_name(cell(headers, cells, "country_name"));
            record.set_daily_retry_history(cell(headers, cells, "daily_retry_history"));
            if (const auto network = cell(headers, cells, "card_network")) {
                CardNetworkValue value;
                value.set_value(*network);
                record.set_card_network(value);
            }
            if (const auto subtype = cell(headers, cells, "payment_method_sub_type")) {
                PaymentMethodTypeValue value;
                value.set_value(*subtype);
                record.set_payment_method_sub_type(value);
            }
            if (const auto active = cell(headers, cells, "is_active")) {
                std::string lowered{};
                for (const char ch : *active) {
                    lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
                }
                if (lowered != "true" && lowered != "false") {
                    CsvParsingError row_error{};
                    row_error.set_row_number(index + 1);
                    row_error.set_error("invalid boolean for is_active: " + *active);
                    failed.push_back(std::move(row_error));
                    continue;
                }
                record.set_is_active(lowered == "true");
            }
            if (const auto history = cell(headers, cells, "account_update_history")) {
                auto decoded = serde::Ser::deserialize<
                    std::vector<::app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord>>(
                    "application/json", *history);
                if (!decoded) {
                    CsvParsingError row_error{};
                    row_error.set_row_number(index + 1);
                    row_error.set_error("invalid account_update_history JSON");
                    failed.push_back(std::move(row_error));
                    continue;
                }
                record.set_account_update_history(std::move(*decoded));
            }
            records.push_back(std::move(record));
        }
        result.set_records(std::move(records));
        result.set_failed_records(std::move(failed));
        return result;
    }

    [[nodiscard]] std::optional<::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult>
    parse_retry_stats_csv(std::string_view csv, std::string& error) const
    {
        using namespace ::app::dto::revenue_recovery_data_backfill;
        RetryStatsMigrationCsvResult result{};
        const auto rows = csv_rows(csv);
        if (rows.empty()) {
            // validate_and_get_records: no rows -> Ok(empty result), never an error.
            return result;
        }
        auto headers = split_csv_line(rows.front().second);
        if (!headers.empty() && headers.front().starts_with("\xEF\xBB\xBF")) {
            headers.front().erase(0, 3);
        }
        std::vector<std::pair<std::size_t, RetryStatsMigrationRecord>> records{};
        std::vector<std::string> errors{};
        for (std::size_t index = 1; index < rows.size(); ++index) {
            const auto cells = split_csv_line(rows[index].second);
            const auto row_number = index + 1; // CSV header is row 1 (api_models:358).
            if (cells.size() != headers.size()) {
                errors.push_back("row " + std::to_string(row_number) + ": found "
                                 + std::to_string(cells.size()) + " fields but header has "
                                 + std::to_string(headers.size()));
                continue;
            }
            const auto cluster_key = cell(headers, cells, "cluster_key");
            const auto stats = cell(headers, cells, "stats");
            if (!cluster_key || !stats) {
                errors.push_back("row " + std::to_string(row_number)
                                 + ": missing cluster_key or stats");
                continue;
            }
            RetryStatsMigrationRecord record{};
            record.set_cluster_key(*cluster_key);
            record.set_stats(*stats);
            records.emplace_back(row_number, std::move(record));
        }
        if (!errors.empty()) {
            const auto total = errors.size();
            std::string message = std::to_string(total) + " row(s) failed CSV parsing: ";
            for (std::size_t i = 0; i < std::min<std::size_t>(total, std::size_t{10}); ++i) {
                if (i > 0) {
                    message += "; ";
                }
                message += errors[i];
            }
            if (total > std::size_t{10}) {
                message += "; ...and " + std::to_string(total - std::size_t{10}) + " more";
            }
            error = message;
            return std::nullopt;
        }
        result.set_records(std::move(records));
        return result;
    }

    // ── wire helpers ──────────────────────────────────────────────────────────

    [[nodiscard]] static std::string join(const std::vector<std::string>& values,
                                          std::string_view separator)
    {
        std::string out{};
        for (const auto& value : values) {
            if (!out.empty()) {
                out.append(separator);
            }
            out.append(value);
        }
        return out;
    }

    void send_error(interfaces::io::IResponse& res, std::function<void()> send,
                    core::error::AppError error) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    /** Route-level `HttpResponse::BadRequest().json({"error": ...})`. */
    void send_bad_request(interfaces::io::IResponse& res, std::function<void()> send,
                          std::string message) const
    {
        serde::Value::Object object{};
        object["error"] = serde::Value{std::move(message)};
        serde::Value document{std::move(object)};
        send_json(res, send, serde::Ser::encode_json(document), false);
    }

    /** Upstream `ErrorResponse{type, code, message}` from ApiErrorResponse. */
    void send_api_error(interfaces::io::IResponse& res, std::function<void()> send,
                        interfaces::io::types::Status status, std::string_view type,
                        std::string_view code, std::string message) const
    {
        serde::Value::Object object{};
        object["type"] = serde::Value{std::string{type}};
        object["code"] = serde::Value{std::string{code}};
        object["message"] = serde::Value{std::move(message)};
        serde::Value document{std::move(object)};
        auto bytes = serde::Ser::serialize_raw("application/json",
                                               serde::Ser::encode_json(document));
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(interfaces::io::IResponse& res, std::function<void()> send,
                   std::string_view text, bool ok) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::revenue_recovery_data_backfill
