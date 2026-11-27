export module app_payment:revenue_recovery_data_backfill_runtime_common;

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

import :revenue_recovery_data_backfill_input;

export namespace app::handlers::revenue_recovery_data_backfill {

namespace backfill_support {

    /** `types/storage/revenue_recovery_redis_operation.rs:21-31`. */
    struct PaymentProcessorTokenDetails
    {
#define RRDB_FIELD(name, ...)                                                                      \
    __VA_ARGS__ name{};                                                                            \
    void set_##name(__VA_ARGS__ value)                                                             \
    {                                                                                              \
        name = std::move(value);                                                                   \
    }                                                                                              \
    [[nodiscard]] const __VA_ARGS__& get_##name() const noexcept                                   \
    {                                                                                              \
        return name;                                                                               \
    }
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
        using History =
            std::vector<::app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord>;
#define RRDB_FIELD(name, ...)                                                                      \
    __VA_ARGS__ name{};                                                                            \
    void set_##name(__VA_ARGS__ value)                                                             \
    {                                                                                              \
        name = std::move(value);                                                                   \
    }                                                                                              \
    [[nodiscard]] const __VA_ARGS__& get_##name() const noexcept                                   \
    {                                                                                              \
        return name;                                                                               \
    }
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
        explicit LocalRedis(core::cache::Cache& cache) noexcept :
            m_cache{cache}
        {
        }

        [[nodiscard]] std::string status_key(std::string_view customer_id) const
        {
            return "customer:" + std::string{customer_id} + ":status";
        }

        [[nodiscard]] std::string tokens_key(std::string_view customer_id) const
        {
            return "customer:" + std::string{customer_id} + ":tokens";
        }

        [[nodiscard]] std::string scoped(std::string_view redis_key) const
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
            m_cache.put(
                key,
                core::cache::to_cache_value(std::string{value}),
                core::cache::Ttl{}.seconds(ttl_seconds)
            );
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
            for (const auto& [token, value]: *object) {
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
            for (const auto& [token, status]: fields) {
                object[token] = serde::Value{status};
            }
            serde::Value document{std::move(object)};
            m_cache.put(
                scoped(redis_key),
                core::cache::to_cache_value(serde::Ser::encode_json(document)),
                core::cache::Ttl{}.seconds(ttl_seconds)
            );
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

    struct ValidatedRow
    {
        std::string key{};
        std::string stats{};
        StatsDocument document{};
    };

} // namespace backfill_support
} // namespace app::handlers::revenue_recovery_data_backfill

// Declared before the support class so encode/decode instantiation sees the
// exact upstream field names rather than an implicitly generated mapping.
template<>
struct serde::Serializable<::app::handlers::revenue_recovery_data_backfill::backfill_support::
                               PaymentProcessorTokenDetails>;
template<>
struct serde::Serializable<
    ::app::handlers::revenue_recovery_data_backfill::backfill_support::PaymentProcessorTokenStatus>;

export namespace app::handlers::revenue_recovery_data_backfill {

/**
 * @brief Shared business helpers for the four revenue-recovery sub-runtimes.
 *
 * Mirrors the original facade's private core helpers: token-status codec, local
 * Redis shadow, card-data assembly, retry-stats validation and persistence, and
 * the metrics flow recorder. No HTTP type appears here.
 */
class RevenueRecoveryRuntimeSupport
{
public:
    using BackfillRequest =
        ::app::dto::revenue_recovery_data_backfill::RevenueRecoveryBackfillRequest;
    using CardData = ::app::dto::revenue_recovery_data_backfill::ComprehensiveCardData;
    using UpdateRequest = ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusRequest;

    explicit RevenueRecoveryRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    void record_flow(
        std::string_view flow,
        std::string_view counter,
        std::size_t processed,
        std::size_t failed
    ) const
    {
        m_context.logger().record_flow(flow);
        m_context.logger().counter(std::string{counter} + ".requests").increment();
        m_context.logger()
            .counter(std::string{counter} + ".processed_records")
            .increment(processed);
        m_context.logger().counter(std::string{counter} + ".failed_records").increment(failed);
    }

    [[nodiscard]] std::int64_t lock_ttl_seconds() const noexcept
    {
        return 3'600;
    }

    [[nodiscard]] std::int64_t redis_ttl_seconds() const noexcept
    {
        return 3'600;
    }

    [[nodiscard]] std::string status_lock_key(std::string_view customer_id) const
    {
        return "revenue_recovery.status:" + std::string{customer_id};
    }

    [[nodiscard]] std::string lock_scoped_key(std::string_view key) const
    {
        return "lock:" + std::string{key};
    }

    [[nodiscard]] std::string
    join(const std::vector<std::string>& values, std::string_view separator) const
    {
        std::string out{};
        for (const auto& value: values) {
            if (!out.empty()) {
                out.append(separator);
            }
            out.append(value);
        }
        return out;
    }

    /**
     * Builds the typed operation error for upstream ApiErrorResponse codes. The
     * upstream code (IR_37, HE_00, IR_48, ...) is preserved in `kind_name` so the
     * serialized `error` field keeps it; `get_status()` comes from the AppErrorKind.
     */
    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message, std::string wire_code) const
    {
        auto error = core::error::AppErrorBuilder{}(kind, std::move(message));
        error.set_kind_name(std::move(wire_code));
        return error;
    }

    [[nodiscard]] std::string
    encode_token_status(const backfill_support::PaymentProcessorTokenStatus& s) const
    {
        return serde::Ser::encode_json(s);
    }

    /**
     * Decodes a stored token status like upstream serde: unknown fields are ignored and
     * every optional field may be absent. Generic navigation keeps the local Redis
     * substitute tolerant of partially populated token records.
     */
    [[nodiscard]] std::optional<backfill_support::PaymentProcessorTokenStatus>
    decode_token_status(std::string_view text) const
    {
        auto doc = serde::Ser::decode_generic("application/json", text);
        if (!doc) {
            return std::nullopt;
        }
        const auto* root = std::get_if<serde::Value::Object>(&doc->get());
        if (root == nullptr) {
            return std::nullopt;
        }
        const auto read_string = [&](const serde::Value::Object& object,
                                     std::string_view key) -> std::optional<std::string>
        {
            const auto found = object.get(std::string{key});
            if (!found) {
                return std::nullopt;
            }
            if (const auto* text = std::get_if<std::string>(&found->get())) {
                return *text;
            }
            return std::nullopt;
        };
        const auto read_bool = [&](const serde::Value::Object& object,
                                   std::string_view key) -> std::optional<bool>
        {
            const auto found = object.get(std::string{key});
            if (!found) {
                return std::nullopt;
            }
            if (const auto* value = std::get_if<bool>(&found->get())) {
                return *value;
            }
            return std::nullopt;
        };

        backfill_support::PaymentProcessorTokenStatus status{};
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
                for (const auto& [key, value]: *history) {
                    if (const auto* number = std::get_if<std::int64_t>(&value.get())) {
                        status.daily_retry_history[key] = static_cast<std::int32_t>(*number);
                    } else if (const auto* number = std::get_if<double>(&value.get())) {
                        status.daily_retry_history[key] = static_cast<std::int32_t>(*number);
                    }
                }
            }
        }
        if (const auto history_found = root->get(std::string{"account_update_history"})) {
            auto history = serde::Ser::from_value<std::vector<
                ::app::dto::revenue_recovery_data_backfill::AccountUpdateHistoryRecord>>(
                *history_found
            );
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

    [[nodiscard]] std::string update_payload_fields(
        std::string_view payload,
        const UpdateRequest* request,
        const std::optional<ScheduledUpdate>& scheduled
    ) const
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

    void persist_token(
        const BackfillRequest& record,
        const std::string& card_json,
        const std::optional<backfill_support::PrimitiveDateTime>& cutoff_datetime
    ) const
    {
        using TokenRecord = ::app::store::revenue_recovery_tokens::RevenueRecoveryTokenRecord;
        auto repository = m_context.repository();
        const auto& token = record.get_token();

        TokenRecord token_record{};
        if (token && !token->empty()) {
            token_record =
                repository.find_revenue_recovery_token_by_string(*token).value_or(TokenRecord{});
        }
        token_record.set_token(token.value_or(""));
        token_record.set_merchant_id(record.get_customer_id_resp());
        token_record.set_payment_id(record.get_connector_payment_id().value_or(""));
        token_record.set_status("active");
        token_record.set_payload(card_json);
        token_record.set_expires_at(
            cutoff_datetime ? cutoff_datetime->storage_string() : std::string{}
        );
        if (token_record.get_created_at().empty()) {
            token_record.set_created_at(
                std::to_string(
                    std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                    )
                        .count()
                )
            );
        }
        if (token_record.get_id() == 0) {
            (void)repository.save_revenue_recovery_token(token_record);
        } else {
            (void)repository.update_revenue_recovery_token(token_record);
        }
    }

    // ── core/revenue_recovery_data_backfill.rs:23-64 / :263-333 ───────────────

    [[nodiscard]] bool process_payment_method_record(
        const BackfillRequest& record,
        const std::optional<backfill_support::PrimitiveDateTime>& cutoff_datetime
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
                    record.get_customer_id_resp(),
                    *token,
                    card_data,
                    cutoff_datetime
                )) {
                return false;
            }
            // Local DB shadow of the Redis token record (durability for the substitute).
            persist_token(record, card_json, cutoff_datetime);
            // Stand-in for the status lock created by the upstream recovery flow; also
            // preserves the key contract consumed by revenue_recovery_redis.cppm.
            if (const auto& payment_id = record.get_connector_payment_id();
                payment_id && !payment_id->empty()) {
                (void)m_context.locks().acquire(
                    status_lock_key(record.get_customer_id_resp()),
                    lock_ttl_seconds(),
                    *payment_id
                );
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
        const std::optional<backfill_support::PrimitiveDateTime>& cutoff_datetime
    ) const
    {
        backfill_support::LocalRedis redis{m_context.cache()};
        const auto key = redis.tokens_key(customer_id);
        auto tokens = redis.get_hash_fields(key);
        auto it = tokens.find(token);
        if (it == tokens.end()) {
            // Bridge: seed the local Redis hash from the DB shadow so a token
            // persisted by the earlier C++ path behaves like an existing Redis token.
            auto repository = m_context.repository();
            auto shadow = repository.find_revenue_recovery_token_by_string(token);
            if (!shadow || shadow->get_merchant_id() != customer_id) {
                return false; // core:1162-1170 ValueNotFound -> failed record.
            }
            backfill_support::PaymentProcessorTokenStatus seeded{};
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
            const auto scheduled =
                backfill_support::PrimitiveDateTime::parse(*status->scheduled_at);
            if (cutoff_datetime && scheduled && *scheduled < *cutoff_datetime) {
                status->scheduled_at = std::nullopt;
            }
        }
        status->modified_at =
            backfill_support::PrimitiveDateTime::now().storage_string(); // core:1221
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
            for (const char ch: value) {
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
        card_data.set_daily_retry_history(
            parse_daily_retry_history(record.get_daily_retry_history())
        );
        return card_data;
    }

    [[nodiscard]] bool parse_expiration_date(
        const std::optional<std::string>& raw,
        std::string& month,
        std::string& year
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
        const auto trim = [](std::string_view part)
        {
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
        for (const char ch: parsed_month) {
            if (ch < '0' || ch > '9') {
                return false;
            }
        }
        const auto month_number =
            backfill_support::PrimitiveDateTime::read_uint(parsed_month, 0, parsed_month.size());
        if (!month_number || *month_number < std::uint32_t{1} ||
            *month_number > std::uint32_t{12}) {
            return false;
        }
        if (parsed_year.size() == std::size_t{4}) {
            for (const char ch: parsed_year) {
                if (ch < '0' || ch > '9') {
                    return false;
                }
            }
            year = parsed_year.substr(2);
        } else if (parsed_year.size() == std::size_t{2}) {
            for (const char ch: parsed_year) {
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
        for (const auto& [key, value]: *object) {
            const auto parsed = backfill_support::PrimitiveDateTime::parse(key);
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
            segments.push_back(rest.substr(
                start,
                pos == std::string_view::npos ? std::string_view::npos : pos - start
            ));
            if (pos == std::string_view::npos) {
                break;
            }
            start = pos + 1;
        }
        if (segments.size() != std::size_t{3}) {
            return std::nullopt;
        }
        for (const auto& segment: segments) {
            for (std::size_t i = 0; i < segment.size(); ++i) {
                if (segment[i] != '%') {
                    continue;
                }
                if (i + std::size_t{2} >= segment.size() ||
                    !std::isxdigit(static_cast<unsigned char>(segment[i + 1])) ||
                    !std::isxdigit(static_cast<unsigned char>(segment[i + 2]))) {
                    return std::nullopt;
                }
                i += 2;
            }
        }
        return std::string{raw};
    }

    [[nodiscard]] std::optional<backfill_support::StatsDocument>
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
        backfill_support::StatsDocument document{};
        for (const auto* name: {"dow", "dom", "hod"}) {
            const auto found = object->get(std::string{name});
            std::vector<backfill_support::SlotCounter>* target = nullptr;
            if (std::string_view{name} == "dow") {
                target = &document.dow;
            } else if (std::string_view{name} == "dom") {
                target = &document.dom;
            } else {
                target = &document.hod;
            }
            const std::size_t expected_slots = std::string_view{name} == "dow"   ? std::size_t{7}
                                               : std::string_view{name} == "dom" ? std::size_t{31}
                                                                                 : std::size_t{24};
            if (!found) {
                // #[serde(default)] -> all-zero array of the fixed length.
                target->resize(expected_slots, backfill_support::SlotCounter{});
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
            for (const auto& element: *array) {
                const auto* slot = std::get_if<serde::Value::Object>(&element.get());
                if (slot == nullptr) {
                    error = std::string{name} + " contains a non-object slot";
                    return std::nullopt;
                }
                backfill_support::SlotCounter counter{};
                const auto read = [&](std::string_view field, std::uint64_t& out)
                {
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
        const auto fill =
            [](std::vector<backfill_support::SlotCounter>& family, std::size_t expected)
        {
            family.resize(expected, backfill_support::SlotCounter{});
        };
        fill(document.dow, std::size_t{7});
        fill(document.dom, std::size_t{31});
        fill(document.hod, std::size_t{24});
        const auto totals =
            [](const std::vector<backfill_support::SlotCounter>& family, bool successes)
        {
            std::uint64_t sum{0};
            for (const auto& slot: family) {
                sum += successes ? slot.k : slot.n;
            }
            return sum;
        };
        for (const auto* family: {&document.dow, &document.dom, &document.hod}) {
            for (const auto& slot: *family) {
                if (slot.k > slot.n) {
                    error = "slot counter has k>n";
                    return std::nullopt;
                }
            }
        }
        const auto [dow_n, dom_n, hod_n] = std::tuple{
            totals(document.dow, false),
            totals(document.dom, false),
            totals(document.hod, false)
        };
        if (dow_n != dom_n || dom_n != hod_n) {
            error = std::format(
                "n totals differ across slot families (dow={}, dom={}, hod={})",
                dow_n,
                dom_n,
                hod_n
            );
            return std::nullopt;
        }
        const auto [dow_k, dom_k, hod_k] = std::tuple{
            totals(document.dow, true),
            totals(document.dom, true),
            totals(document.hod, true)
        };
        if (dow_k != dom_k || dom_k != hod_k) {
            error = std::format(
                "k totals differ across slot families (dow={}, dom={}, hod={})",
                dow_k,
                dom_k,
                hod_k
            );
            return std::nullopt;
        }
        return document;
    }

    /** Port of record::replace_retry_stats_document (retry_stats/record.rs:79-108, 112-142). */
    [[nodiscard]] std::optional<bool>
    replace_retry_stats_document(const std::string& cluster_key, std::string_view stats) const
    {
        backfill_support::LocalRedis redis{m_context.cache()};
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
        const bool existed =
            m_context.config_store().get("revenue_recovery_retry_stats", cluster_key).has_value();
        m_context.config_store().put(
            "revenue_recovery_retry_stats",
            cluster_key,
            std::string{stats}
        ); // whole-document replace.
        // Release only while still the owner (record.rs:270-284).
        if (redis.get_key(lock_key) == std::optional<std::string>{"migration"}) {
            redis.delete_key(lock_key);
        }
        return !existed; // Ok(true) - upstream ignores insert/update distinction here.
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::revenue_recovery_data_backfill

// rfl reflection for the local token-status storage record (declared with the type's
// partition so decode/encode instantiation sees the exact upstream field names).
namespace serde {
template<>
struct Serializable<
    ::app::handlers::revenue_recovery_data_backfill::backfill_support::PaymentProcessorTokenDetails>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::revenue_recovery_data_backfill::backfill_support::
            PaymentProcessorTokenDetails;
        return std::tuple{
            serde::FieldDesc<
                "payment_processor_token",
                &T::get_payment_processor_token,
                &T::set_payment_processor_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_month",
                &T::get_expiry_month,
                &T::set_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_year",
                &T::get_expiry_year,
                &T::set_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_issuer",
                &T::get_card_issuer,
                &T::set_card_issuer,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "last_four_digits",
                &T::get_last_four_digits,
                &T::set_last_four_digits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &T::get_card_network,
                &T::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &T::get_card_type,
                &T::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_isin",
                &T::get_card_isin,
                &T::set_card_isin,
                serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_processor_token_details";
    }
};

template<>
struct ::serde::Serializable<
    ::app::handlers::revenue_recovery_data_backfill::backfill_support::PaymentProcessorTokenStatus>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::revenue_recovery_data_backfill::backfill_support::
            PaymentProcessorTokenStatus;
        return std::tuple{
            serde::FieldDesc<
                "payment_processor_token_details",
                &T::get_payment_processor_token_details,
                &T::set_payment_processor_token_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "inserted_by_attempt_id",
                &T::get_inserted_by_attempt_id,
                &T::set_inserted_by_attempt_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &T::get_error_code,
                &T::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "daily_retry_history",
                &T::get_daily_retry_history,
                &T::set_daily_retry_history,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "scheduled_at",
                &T::get_scheduled_at,
                &T::set_scheduled_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_hard_decline",
                &T::get_is_hard_decline,
                &T::set_is_hard_decline,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &T::get_modified_at,
                &T::set_modified_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_active",
                &T::get_is_active,
                &T::set_is_active,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_update_history",
                &T::get_account_update_history,
                &T::set_account_update_history,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "decision_threshold",
                &T::get_decision_threshold,
                &T::set_decision_threshold,
                serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payment_processor_token_status";
    }
};
} // namespace serde
