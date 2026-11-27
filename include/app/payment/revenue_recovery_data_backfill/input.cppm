export module app_payment:revenue_recovery_data_backfill_input;

import std;
import core_behavior;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

export namespace app::handlers::revenue_recovery_data_backfill {

namespace backfill_support {

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
        std::int64_t year{1'970};
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
            if (!month || !day || *month < std::uint32_t{1} || *month > std::uint32_t{12} ||
                *day < std::uint32_t{1} || *day > std::uint32_t{31}) {
                return std::nullopt;
            }
            PrimitiveDateTime out{};
            out.year = static_cast<std::int64_t>(*year);
            out.month = *month;
            out.day = *day;
            if (text.size() == std::size_t{10}) {
                return out; // Date::parse -> Time::MIDNIGHT fallback.
            }
            if (text.size() < std::size_t{19} || (text[10] != ' ' && text[10] != 'T') ||
                text[13] != ':' || text[16] != ':') {
                return std::nullopt;
            }
            const auto hour = read_uint(text, 11, 2);
            const auto minute = read_uint(text, 14, 2);
            const auto second = read_uint(text, 17, 2);
            if (!hour || !minute || !second || *hour > std::uint32_t{23} ||
                *minute > std::uint32_t{59} || *second > std::uint32_t{59}) {
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

        /** `time::format_description::well_known::Iso8601::DEFAULT` for BackfillQuery.cutoff_time.
         */
        [[nodiscard]] static std::optional<PrimitiveDateTime>
        parse_iso8601(std::string_view text) noexcept
        {
            const auto year = read_uint(text, 0, 4);
            if (!year || text.size() < std::size_t{19} || text[4] != '-' || text[7] != '-' ||
                (text[10] != 'T' && text[10] != ' ')) {
                return std::nullopt;
            }
            const auto month = read_uint(text, 5, 2);
            const auto day = read_uint(text, 8, 2);
            const auto hour = read_uint(text, 11, 2);
            const auto minute = read_uint(text, 14, 2);
            const auto second = read_uint(text, 17, 2);
            if (!month || !day || !hour || !minute || !second || *month < std::uint32_t{1} ||
                *month > std::uint32_t{12} || *day < std::uint32_t{1} || *day > std::uint32_t{31} ||
                *hour > std::uint32_t{23} || *minute > std::uint32_t{59} ||
                *second > std::uint32_t{59}) {
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
            const std::int64_t days = seconds / std::int64_t{86'400};
            const std::int64_t seconds_of_day = seconds % std::int64_t{86'400};
            // Howard Hinnant's civil_from_days.
            const std::int64_t z = days + std::int64_t{719'468};
            const std::int64_t era =
                (z >= 0 ? z : z - std::int64_t{146'096}) / std::int64_t{146'097};
            const std::int64_t doe = z - era * std::int64_t{146'097};
            const std::int64_t yoe = (doe - doe / std::int64_t{1'460} + doe / std::int64_t{36'524} -
                                      doe / std::int64_t{146'096}) /
                                     std::int64_t{365};
            std::int64_t year = yoe + era * std::int64_t{400};
            const std::int64_t doy =
                doe - (std::int64_t{365} * yoe + yoe / std::int64_t{4} - yoe / std::int64_t{100});
            const std::int64_t mp = (std::int64_t{5} * doy + std::int64_t{2}) / std::int64_t{153};
            const std::int64_t day = doy -
                                     (std::int64_t{153} * mp + std::int64_t{2}) / std::int64_t{5} +
                                     std::int64_t{1};
            const std::int64_t month =
                mp + (mp < std::int64_t{10} ? std::int64_t{3} : -std::int64_t{9});
            year += (month <= std::int64_t{2}) ? std::int64_t{1} : std::int64_t{0};
            PrimitiveDateTime out{};
            out.year = year;
            out.month = static_cast<std::uint32_t>(month);
            out.day = static_cast<std::uint32_t>(day);
            out.hour = static_cast<std::uint32_t>(seconds_of_day / std::int64_t{3'600});
            out.minute = static_cast<std::uint32_t>(
                (seconds_of_day % std::int64_t{3'600}) / std::int64_t{60}
            );
            out.second = static_cast<std::uint32_t>(seconds_of_day % std::int64_t{60});
            return out;
        }

        /** Upstream Redis-key / CSV-key datetime layout. */
        [[nodiscard]] std::string storage_string() const
        {
            return std::format(
                "{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:06}",
                year,
                month,
                day,
                hour,
                minute,
                second,
                micro
            );
        }
    };

} // namespace backfill_support

enum class ScheduledKind
{
    Null,
    DateTime
};

struct ScheduledUpdate
{
    ScheduledKind kind{ScheduledKind::Null};
    std::string datetime{};
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received.

struct RevenueRecoveryDataBackfillInput
{
    ::app::dto::revenue_recovery_data_backfill::CsvParsingResult parsed{};
    std::optional<backfill_support::PrimitiveDateTime> cutoff_datetime{};
};

struct RetryStatsMigrationInput
{
    ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult parsed{};
};

struct UpdateRedisDataInput
{
    ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest parsed{};
    std::optional<ScheduledUpdate> scheduled_update{};
};

struct UnlockStatusInput
{
    ::app::dto::revenue_recovery_data_backfill::UnlockStatusRequest parsed{};
};

/**
 * @brief Parse/auth/path/query support for the revenue-recovery handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime. Route-level parse failures are returned
 * as a message string so the handler can wrap them in a typed AppError.
 */
class RevenueRecoveryDataBackfillInputSupport
{
public:
    explicit RevenueRecoveryDataBackfillInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::expected<std::optional<backfill_support::PrimitiveDateTime>, std::string>
    cutoff_datetime_from_query(interfaces::io::IRequest& req) const
    {
        const auto query = ::app::http::RequestQuery{}(req);
        const auto cutoff_time = query.get("cutoff_time");
        std::optional<backfill_support::PrimitiveDateTime> cutoff_datetime{};
        if (cutoff_time && !cutoff_time->empty()) {
            cutoff_datetime = backfill_support::PrimitiveDateTime::parse_iso8601(*cutoff_time);
            if (!cutoff_datetime) {
                return std::unexpected{std::string{"Invalid datetime format: invalid datetime "
                                                   "format. Use ISO8601: 2024-01-15T10:30:00"}};
            }
        }
        return cutoff_datetime;
    }

    [[nodiscard]] bool is_json_body(std::string_view body) const
    {
        const auto first = body.find_first_not_of(" \t\r\n");
        return first != std::string_view::npos && (body[first] == '{' || body[first] == '[');
    }

    /** Extract the `file` part of a multipart body; pass raw CSV through unchanged. */
    [[nodiscard]] std::string unwrap_multipart_file(std::string_view body) const
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
        std::string out{
            body.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start)
        };
        while (!out.empty() && (out.back() == '\r' || out.back() == '\n')) {
            out.pop_back();
        }
        return out;
    }

    /** Upstream ScheduledAtUpdate custom Deserializer (api_models:228-253). */
    [[nodiscard]] std::expected<std::optional<ScheduledUpdate>, std::string>
    parse_scheduled_update(const serde::Value::Object& root) const
    {
        std::optional<ScheduledUpdate> scheduled_update{};
        const auto found = root.get(std::string{"scheduled_at"});
        if (!found) {
            return scheduled_update;
        }
        if (const auto* text = std::get_if<std::string>(&found->get())) {
            std::string lowered{};
            lowered.reserve(text->size());
            for (const char ch: *text) {
                lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            }
            if (lowered == "null") {
                scheduled_update = ScheduledUpdate{ScheduledKind::Null, std::string{}};
            } else if (const auto dt = backfill_support::PrimitiveDateTime::parse_iso8601(*text)) {
                scheduled_update = ScheduledUpdate{ScheduledKind::DateTime, dt->storage_string()};
            } else {
                return std::unexpected{
                    std::string{"Invalid scheduled_at: expected null or datetime iso8601"}
                };
            }
        } else {
            return std::unexpected{
                std::string{"Expected null variable or datetime iso8601 for scheduled_at"}
            };
        }
        return scheduled_update;
    }

    [[nodiscard]] std::expected<
        ::app::dto::revenue_recovery_data_backfill::UnlockStatusRequest,
        core::error::AppError>
    status_request_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        using Request = ::app::dto::revenue_recovery_data_backfill::UnlockStatusRequest;
        Request parsed;
        if (segments.count() < 2) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector customer id or payment intent id in path"
            )};
        }
        parsed.set_connector_customer_id(
            std::string{segments.at(segments.count() - 2).value_or("")}
        );
        ::app::dto::revenue_recovery_data_backfill::GlobalPaymentIdValue payment_id;
        payment_id.set_value(std::string{segments.at(segments.count() - 1).value_or("")});
        parsed.set_payment_intent_id(std::move(payment_id));
        return parsed;
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
                row_error.set_error(
                    "found record with " + std::to_string(cells.size()) +
                    " fields but header has " + std::to_string(headers.size())
                );
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
                for (const char ch: *active) {
                    lowered.push_back(
                        static_cast<char>(std::tolower(static_cast<unsigned char>(ch)))
                    );
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
                auto decoded = serde::Ser::deserialize<std::vector<
                    ::app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord>>(
                    "application/json",
                    *history
                );
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

    [[nodiscard]] std::optional<
        ::app::dto::revenue_recovery_data_backfill::RetryStatsMigrationCsvResult>
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
                errors.push_back(
                    "row " + std::to_string(row_number) + ": found " +
                    std::to_string(cells.size()) + " fields but header has " +
                    std::to_string(headers.size())
                );
                continue;
            }
            const auto cluster_key = cell(headers, cells, "cluster_key");
            const auto stats = cell(headers, cells, "stats");
            if (!cluster_key || !stats) {
                errors.push_back(
                    "row " + std::to_string(row_number) + ": missing cluster_key or stats"
                );
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

private:
    [[nodiscard]] std::vector<std::string> split_csv_line(std::string_view line) const
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

    [[nodiscard]] std::vector<std::pair<std::size_t, std::string>>
    csv_rows(std::string_view csv) const
    {
        std::vector<std::pair<std::size_t, std::string>> rows{};
        std::size_t row_number{1};
        std::size_t start{0};
        while (start <= csv.size()) {
            const auto newline = csv.find('\n', start);
            std::string_view line = csv.substr(
                start,
                newline == std::string_view::npos ? std::string_view::npos : newline - start
            );
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

    [[nodiscard]] std::optional<std::string> cell(
        const std::vector<std::string>& headers,
        const std::vector<std::string>& cells,
        std::string_view name
    ) const
    {
        for (std::size_t i = 0; i < headers.size(); ++i) {
            if (headers[i] == name && i < cells.size() && !cells[i].empty()) {
                return cells[i];
            }
        }
        return std::nullopt;
    }

    ::app::context::AppContext& m_context;
};

/** @brief Request policy for the revenue_recovery_data_backfill domain: wraps the domain auth
 * rules. */
class RevenueRecoveryDataBackfillPolicy
{
public:
    RevenueRecoveryDataBackfillPolicy(
        ::app::context::AppContext& /*context*/,
        RevenueRecoveryDataBackfillInputSupport& support
    ) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        return m_support.authorize(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        core::behavior::Scope result;
        result.merchant_id = std::string{auth.merchant_id()};
        result.profile_id = std::string{auth.profile_id()};
        result.org_id = std::string{auth.org_id()};
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        result.connected_merchant_id = connected ? std::string{*connected} : std::string{};
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const
    {
        return std::nullopt;
    }


private:
    RevenueRecoveryDataBackfillInputSupport& m_support;
};

using RevenueRecoveryDataBackfillRequestSupport =
    core::behavior::RequestSupport<RevenueRecoveryDataBackfillPolicy>;
using RevenueRecoveryDataBackfillPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] RevenueRecoveryDataBackfillPolicySpec spec_for(std::string_view operation) noexcept
{
    return RevenueRecoveryDataBackfillPolicySpec{.permission = operation};
}
} // namespace app::handlers::revenue_recovery_data_backfill
