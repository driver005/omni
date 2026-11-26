export module app_webhook:webhook_events_runtime_common;

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

export namespace app::handlers::webhook_events {

using Record = ::app::store::webhook_events::WebhookEventRecord;
using Constraints = ::app::dto::webhook_events::EventListConstraints;
using InternalKind = ::app::dto::webhook_events::EventListConstraintsInternal;
using EventListResult = ::app::dto::webhook_events::EventListResult;

// Port of api_models::webhook_events::EventListConstraintsInternal.
struct InternalConstraints
{
    InternalKind kind{InternalKind::GenericFilter};
    // ObjectIdFilter
    std::string object_id{};
    // EventIdFilter
    std::string event_id{};
    // GenericFilter
    std::optional<std::string> created_after{};
    std::optional<std::string> created_before{};
    std::optional<std::int64_t> limit{};
    std::optional<std::int64_t> offset{};
    std::vector<std::string> event_classes{};
    std::vector<std::string> event_types{};
    std::optional<bool> is_delivered{};
    std::optional<std::string> recipient{};
};

enum class DeliverySource
{
    ListInitialEvents,
    ListDeliveryAttempts,
};

/**
 * @brief Business helpers shared by the webhook_events sub-runtimes.
 *
 * In-process substitutes for the upstream event store, profile config, key
 * manager and id generators. No HTTP types live in this partition.
 */
class WebhookEventsRuntimeSupport
{
public:
    explicit WebhookEventsRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // Local in-process substitute for find_business_profile_by_merchant_id_profile_id
    // / find_business_profile_by_profile_id (core:30-53, :344-353).
    [[nodiscard]] std::optional<core::error::AppError>
    validate_profile(const std::string& profile_id, const std::string& merchant_id) const
    {
        if (profile_id.empty()) {
            return std::nullopt;
        }
        const auto stored = m_context.config_store().get_value("business_profile", profile_id);
        if (!stored) {
            m_context.metrics().counter("WEBHOOK_EVENT_PROFILE_UNRESOLVED").increment();
            return std::nullopt;
        }
        if (!merchant_id.empty() && *stored != merchant_id) {
            return error(
                core::error::AppErrorKind::FORBIDDEN,
                "business profile does not belong to merchant"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string event_key(const std::string& merchant_id) const
    {
        const std::string key_id = merchant_id.empty() ? std::string{"v1"} : merchant_id;
        if (const auto secret = m_context.key_store().get("webhook_event", key_id)) {
            return secret->value();
        }
        return std::string{"webhook-event-local-substitute"};
    }

    // Deterministic decrypt of the local enc1 framing; plaintext rows pass through.
    [[nodiscard]] std::string payload_of(const Record& record, const std::string& merchant_id) const
    {
        const std::string& value = record.get_payload();
        constexpr std::string_view marker{"enc1:"};
        if (!value.starts_with(marker)) {
            return value;
        }
        const std::string_view encoded = std::string_view{value}.substr(marker.size());
        std::string raw{};
        raw.reserve(encoded.size() / 2U);
        const auto nibble = [](char ch) -> int
        {
            if (ch >= '0' && ch <= '9') {
                return ch - '0';
            }
            if (ch >= 'a' && ch <= 'f') {
                return ch - 'a' + 10;
            }
            if (ch >= 'A' && ch <= 'F') {
                return ch - 'A' + 10;
            }
            return -1;
        };
        for (std::size_t i = 0; i + 1 < encoded.size(); i += 2) {
            const int high = nibble(encoded[i]);
            const int low = nibble(encoded[i + 1]);
            if (high < 0 || low < 0) {
                return value;
            }
            raw.push_back(static_cast<char>((high << 4) | low));
        }
        return ::app::crypto::AesWrap{}.decrypt(raw, event_key(merchant_id));
    }

    [[nodiscard]] std::string payload_field(std::string_view payload, std::string_view key) const
    {
        const std::string needle = std::format("\"{}\":\"", key);
        const auto pos = payload.find(needle);
        if (pos == std::string_view::npos) {
            return {};
        }
        const std::size_t start = pos + needle.size();
        const std::size_t end = payload.find('"', start);
        if (end == std::string_view::npos) {
            return {};
        }
        return std::string{payload.substr(start, end - start)};
    }

    // Boolean fields are stored unquoted in JSON.
    [[nodiscard]] std::optional<bool>
    payload_bool(std::string_view payload, std::string_view key) const
    {
        const std::string needle = std::format("\"{}\":", key);
        const auto pos = payload.find(needle);
        if (pos == std::string_view::npos) {
            return std::nullopt;
        }
        std::size_t cursor = pos + needle.size();
        while (cursor < payload.size() &&
               std::isspace(static_cast<unsigned char>(payload[cursor]))) {
            ++cursor;
        }
        if (payload.substr(cursor, 4) == "true") {
            return true;
        }
        if (payload.substr(cursor, 5) == "false") {
            return false;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch: value) {
            switch (ch) {
                case '\\':
                    out += "\\\\";
                    break;
                case '"':
                    out += "\\\"";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    out.push_back(ch);
                    break;
            }
        }
        return out;
    }

    [[nodiscard]] std::string
    initial_attempt_id_of(const Record& record, std::string_view payload) const
    {
        const std::string stored = payload_field(payload, "initial_attempt_id");
        return stored.empty() ? record.get_event_id() : stored;
    }

    [[nodiscard]] std::string initiator_of(const Record& record, std::string_view payload) const
    {
        const std::string stored = payload_field(payload, "initiator_merchant_id");
        return stored.empty() ? record.get_merchant_id() : stored;
    }

    [[nodiscard]] std::string
    profile_id_of(std::string_view payload, std::string_view fallback = {}) const
    {
        const std::string stored = payload_field(payload, "business_profile_id");
        if (!stored.empty()) {
            return stored;
        }
        const std::string alt = payload_field(payload, "profile_id");
        return alt.empty() ? std::string{fallback} : alt;
    }

    [[nodiscard]] std::string object_id_of(std::string_view payload) const
    {
        return payload_field(payload, "primary_object_id");
    }

    [[nodiscard]] std::string recipient_of(std::string_view payload) const
    {
        const std::string stored = payload_field(payload, "recipient");
        return stored.empty() ? std::string{"merchant"} : stored;
    }

    // Upstream resolve_delivery_success (event.rs:102-107):
    //   ListInitialEvents     -> is_overall_delivery_successful
    //   ListDeliveryAttempts  -> Some(is_webhook_notified)
    [[nodiscard]] std::optional<bool>
    delivery_success_of(const Record& record, std::string_view payload, DeliverySource source) const
    {
        if (source == DeliverySource::ListInitialEvents) {
            const auto overall = payload_bool(payload, "is_overall_delivery_successful");
            if (overall) {
                return overall;
            }
        } else {
            const auto notified = payload_bool(payload, "is_webhook_notified");
            if (notified) {
                return notified;
            }
        }
        // Local fallback for rows seeded without the upstream domain fields.
        const auto legacy = payload_bool(payload, "is_delivered");
        if (legacy) {
            return legacy;
        }
        const auto& status = record.get_status();
        if (status == "DELIVERED" || status == "SUCCESS" || status == "Success") {
            return true;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> parse_seconds(std::string_view value) const
    {
        if (value.empty()) {
            return std::nullopt;
        }
        try {
            std::size_t consumed{0};
            const auto parsed = std::stoll(std::string{value}, &consumed);
            if (consumed == value.size()) {
                return parsed;
            }
        } catch (...) {
        }
        return std::nullopt;
    }

    // Upstream bounds are PrimitiveDateTime deserialized from RFC3339/ISO8601
    // (common_utils::custom_serde::iso8601). Accept both ISO strings and the
    // numeric seconds used by locally seeded records.
    [[nodiscard]] std::optional<std::int64_t> parse_timestamp(std::string_view value) const
    {
        if (const auto numeric = parse_seconds(value)) {
            return numeric;
        }
        if (value.size() < 19) {
            return std::nullopt;
        }
        const auto number = [value](std::size_t pos, std::size_t length) -> std::optional<int>
        {
            int result{0};
            for (std::size_t i{0}; i < length; ++i) {
                if (pos + i >= value.size() ||
                    !std::isdigit(static_cast<unsigned char>(value[pos + i]))) {
                    return std::nullopt;
                }
                result = result * 10 + (value[pos + i] - '0');
            }
            return result;
        };
        const auto year = number(0, 4);
        const auto month = number(5, 2);
        const auto day = number(8, 2);
        const auto hour = number(11, 2);
        const auto minute = number(14, 2);
        const auto second = number(17, 2);
        if (!year || !month || !day || !hour || !minute || !second) {
            return std::nullopt;
        }
        if (value[4] != '-' || value[7] != '-' || (value[10] != 'T' && value[10] != ' ') ||
            value[13] != ':' || value[16] != ':') {
            return std::nullopt;
        }

        // Days since 1970-01-01 (Howard Hinnant's civil-date algorithm).
        int y = *year - (*month <= 2 ? 1 : 0);
        const int era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153 * static_cast<unsigned>(*month + (*month > 2 ? -3 : 9)) + 2) / 5 +
                             static_cast<unsigned>(*day) - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        const std::int64_t days =
            static_cast<std::int64_t>(era) * 146'097 + static_cast<std::int64_t>(doe) - 719'468;
        std::int64_t seconds = days * 86'400 + static_cast<std::int64_t>(*hour) * 3'600 +
                               static_cast<std::int64_t>(*minute) * 60 + *second;

        // Optional timezone offset (Z or ±HH:MM).
        const auto zone = value.find_last_of("Zz+-");
        if (zone != std::string_view::npos && zone > 10 && value[zone] != 'Z' &&
            value[zone] != 'z') {
            const auto offset_hour = number(zone + 1, 2);
            const auto offset_minute = number(zone + 4, 2);
            if (offset_hour && offset_minute) {
                const std::int64_t offset =
                    static_cast<std::int64_t>(*offset_hour) * 3'600 + *offset_minute * 60;
                seconds += value[zone] == '-' ? offset : -offset;
            }
        }
        return seconds;
    }

    // Port of EventListConstraintsInternal::foreign_try_from
    // (transformers.rs:2224-2269).
    [[nodiscard]] std::expected<InternalConstraints, core::error::AppError>
    to_internal(const Constraints& constraints) const
    {
        const bool has_object = constraints.get_object_id().has_value();
        const bool has_event = constraints.get_event_id().has_value();
        const bool has_generic =
            constraints.get_created_after().has_value() ||
            constraints.get_created_before().has_value() || constraints.get_limit().has_value() ||
            constraints.get_offset().has_value() || constraints.get_event_classes().has_value() ||
            constraints.get_event_types().has_value();

        if ((has_object || has_event) && has_generic) {
            return std::unexpected{error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Either `object_id` alone, or `event_id` alone, or one or more of `created_after`, "
                "`created_before`, `limit`, `offset`, `event_classes` and `event_types` must be "
                "specified"
            )};
        }
        if (has_object && has_event) {
            return std::unexpected{error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Cannot specify both `object_id` and `event_id`. Please provide only one."
            )};
        }

        InternalConstraints out{};
        if (has_object) {
            out.kind = InternalKind::ObjectIdFilter;
            out.object_id = *constraints.get_object_id();
            if (constraints.get_recipient()) {
                out.recipient = constraints.get_recipient()->get_value();
            }
            return out;
        }
        if (has_event) {
            out.kind = InternalKind::EventIdFilter;
            out.event_id = *constraints.get_event_id();
            return out;
        }

        out.kind = InternalKind::GenericFilter;
        if (constraints.get_created_after()) {
            out.created_after = constraints.get_created_after()->get_value();
        }
        if (constraints.get_created_before()) {
            out.created_before = constraints.get_created_before()->get_value();
        }
        if (constraints.get_limit()) {
            out.limit = static_cast<std::int64_t>(*constraints.get_limit());
        }
        if (constraints.get_offset()) {
            out.offset = static_cast<std::int64_t>(*constraints.get_offset());
        }
        if (constraints.get_event_classes()) {
            for (const auto& value: *constraints.get_event_classes()) {
                out.event_classes.push_back(value.get_value());
            }
        }
        if (constraints.get_event_types()) {
            for (const auto& value: *constraints.get_event_types()) {
                out.event_types.push_back(value.get_value());
            }
        }
        if (constraints.get_is_delivered()) {
            out.is_delivered = *constraints.get_is_delivered();
        }
        if (constraints.get_recipient()) {
            out.recipient = constraints.get_recipient()->get_value();
        }
        return out;
    }

    // EventClass::event_types() (common_enums/src/enums.rs:2065-2107), snake_case wire values.
    [[nodiscard]] std::vector<std::string> event_types_of_class(std::string_view event_class) const
    {
        if (event_class == "payments") {
            return {
                "payment_succeeded",
                "payment_failed",
                "payment_processing",
                "payment_cancelled",
                "payment_cancelled_post_capture",
                "payment_authorized",
                "payment_captured",
                "payment_expired",
                "action_required",
                "surcharge_payment_succeeded",
            };
        }
        if (event_class == "refunds") {
            return {
                "refund_succeeded",
                "refund_failed",
                "surcharge_refund_succeeded",
                "refund_review"
            };
        }
        if (event_class == "disputes") {
            return {
                "dispute_opened",
                "dispute_expired",
                "dispute_accepted",
                "dispute_cancelled",
                "dispute_challenged",
                "dispute_won",
                "dispute_lost",
            };
        }
        if (event_class == "mandates") {
            return {"mandate_active", "mandate_revoked"};
        }
        if (event_class == "payouts") {
            return {
                "payout_success",
                "payout_failed",
                "payout_initiated",
                "payout_processing",
                "payout_cancelled",
                "payout_expired",
                "payout_reversed",
                "payout_not_permitted",
            };
        }
        if (event_class == "subscriptions") {
            return {"invoice_paid"};
        }
        if (event_class == "revenue_recovery") {
            return {"recovery_retry_scheduled", "recovery_limits_exhausted"};
        }
        return {};
    }

    [[nodiscard]] bool
    contains(const std::vector<std::string>& values, const std::string& needle) const
    {
        return std::find(values.begin(), values.end(), needle) != values.end();
    }

    [[nodiscard]] bool
    disjoint(const std::vector<std::string>& lhs, const std::vector<std::string>& rhs) const
    {
        for (const auto& value: lhs) {
            if (contains(rhs, value)) {
                return false;
            }
        }
        return true;
    }

    // Port of finalize_event_types (core:481-524).
    [[nodiscard]] std::expected<std::vector<std::string>, core::error::AppError>
    finalize_event_types(
        const std::vector<std::string>& event_classes,
        std::vector<std::string> event_types
    ) const
    {
        std::vector<std::string> possible{};
        for (const auto& event_class: event_classes) {
            for (const auto& event_type: event_types_of_class(event_class)) {
                if (!contains(possible, event_type)) {
                    possible.push_back(event_type);
                }
            }
        }

        if (event_types.empty()) {
            return possible;
        }

        // Extend with each class whose event types are disjoint from the request.
        for (const auto& event_class: event_classes) {
            const auto valid = event_types_of_class(event_class);
            if (disjoint(event_types, valid)) {
                for (const auto& event_type: valid) {
                    if (!contains(event_types, event_type)) {
                        event_types.push_back(event_type);
                    }
                }
            }
        }

        for (const auto& event_type: event_types) {
            if (!contains(possible, event_type)) {
                return std::unexpected{error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "`event_types` must be a subset of `event_classes`"
                )};
            }
        }
        return event_types;
    }

    [[nodiscard]] std::string now_value() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            )
                .count()
        );
    }

    // Port of list_initial_delivery_attempts (core:20-259).
    [[nodiscard]] std::expected<EventListResult, core::error::AppError>
    list_initial_delivery_attempts(
        const std::string& merchant_id,
        const std::string& profile_id,
        const Constraints& constraints
    ) const
    {
        auto internal = to_internal(constraints);
        if (!internal) {
            return std::unexpected{std::move(internal.error())};
        }

        EventListResult result{};
        switch (internal->kind) {
            case InternalKind::ObjectIdFilter:
                {
                    // list_initial_events_by_initiator_merchant_id_primary_object_id
                    // (core:60-85): no limit/offset, total_count = events.len().
                    std::vector<std::pair<Record, std::string>> matched{};
                    for (const auto& record: m_context.repository().list_webhook_events(
                             ::app::store::WebhookEventQuery{}.merchant_id(merchant_id)
                         )) {
                        const auto payload = payload_of(record, merchant_id);
                        if (!is_initial_attempt(record, payload)) {
                            continue;
                        }
                        if (initiator_of(record, payload) != merchant_id) {
                            continue;
                        }
                        if (object_id_of(payload) != internal->object_id) {
                            continue;
                        }
                        if (!profile_id.empty() &&
                            profile_id_of(payload, profile_id) != profile_id) {
                            continue;
                        }
                        if (internal->recipient &&
                            !recipient_matches(recipient_of(payload), *internal->recipient)) {
                            continue;
                        }
                        matched.emplace_back(record, payload);
                    }
                    // Upstream orders by created_at DESC.
                    std::stable_sort(
                        matched.begin(),
                        matched.end(),
                        [](const auto& lhs, const auto& rhs)
                        {
                            return lhs.first.get_created_at() > rhs.first.get_created_at();
                        }
                    );
                    result.total_count = static_cast<std::int64_t>(matched.size());
                    for (const auto& [record, payload]: matched) {
                        auto item = to_list_item(
                            record,
                            payload,
                            DeliverySource::ListInitialEvents,
                            profile_id
                        );
                        if (!item) {
                            return std::unexpected{std::move(item.error())};
                        }
                        result.items.push_back(std::move(*item));
                    }
                    return result;
                }
            case InternalKind::EventIdFilter:
                {
                    // find_event_by_event_id + initiator/profile/initial-attempt scoping
                    // (core:87-116): total_count is 0 or 1.
                    const auto found =
                        m_context.repository().find_webhook_event_by_event_id(internal->event_id);
                    if (!found) {
                        return result;
                    }
                    const auto payload = payload_of(*found, merchant_id);
                    const bool matches_initiator = initiator_of(*found, payload) == merchant_id;
                    const bool matches_profile =
                        profile_id.empty() || profile_id_of(payload, profile_id) == profile_id;
                    if (!matches_initiator || !matches_profile ||
                        initial_attempt_id_of(*found, payload) != found->get_event_id()) {
                        return result;
                    }
                    auto item = to_list_item(
                        *found,
                        payload,
                        DeliverySource::ListInitialEvents,
                        profile_id
                    );
                    if (!item) {
                        return std::unexpected{std::move(item.error())};
                    }
                    result.items.push_back(std::move(*item));
                    result.total_count = 1;
                    return result;
                }
            case InternalKind::GenericFilter:
                return list_generic(merchant_id, profile_id, *internal);
        }
        return result;
    }

    [[nodiscard]] bool is_initial_attempt(const Record& record, std::string_view payload) const
    {
        return initial_attempt_id_of(record, payload) == record.get_event_id();
    }

    [[nodiscard]] bool
    recipient_matches(const std::string& stored, const std::string& requested) const
    {
        if (requested == "merchant") {
            // NULL/empty recipient is treated as Merchant for backward compatibility.
            return stored.empty() || stored == "merchant";
        }
        return stored == requested;
    }

    [[nodiscard]] std::expected<EventListResult, core::error::AppError> list_generic(
        const std::string& merchant_id,
        const std::string& profile_id,
        const InternalConstraints& internal
    ) const
    {
        const std::int64_t now = parse_seconds(now_value()).value_or(0);
        const std::int64_t begin_time = (now / 86'400 - 90) * 86'400; // midnight UTC, 90 days ago

        // limit validation (core:123-131).
        std::int64_t limit{100};
        if (internal.limit) {
            if (*internal.limit > 100) {
                return std::unexpected{error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "`limit` must be a number less than 100"
                )};
            }
            limit = *internal.limit;
        }
        std::optional<std::int64_t> offset{};
        if (internal.offset && *internal.offset > 0) {
            offset = *internal.offset;
        }

        std::vector<std::string> event_types = internal.event_types;
        if (!internal.event_classes.empty()) {
            auto resolved = finalize_event_types(internal.event_classes, event_types);
            if (!resolved) {
                return std::unexpected{std::move(resolved.error())};
            }
            event_types = std::move(*resolved);
        }

        // created_after/created_before validation (core:143-173).
        if (internal.created_after && internal.created_before &&
            *internal.created_after > *internal.created_before) {
            return std::unexpected{error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "The `created_after` timestamp must be an earlier timestamp compared to the "
                "`created_before` timestamp"
            )};
        }
        std::int64_t created_after = begin_time;
        if (internal.created_after) {
            const auto parsed = parse_timestamp(*internal.created_after);
            if (!parsed || *parsed < begin_time) {
                return std::unexpected{error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "`created_after` must be a timestamp within the past 90 days."
                )};
            }
            created_after = *parsed;
        }
        std::int64_t created_before = now;
        if (internal.created_before) {
            const auto parsed = parse_timestamp(*internal.created_before);
            if (!parsed || *parsed < begin_time) {
                return std::unexpected{error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "`created_before` must be a timestamp within the past 90 days."
                )};
            }
            created_before = *parsed;
        }

        // The profile variant filters by business_profile_id; the merchant variant
        // filters by initiator_merchant_id with the legacy merchant_id fallback.
        std::vector<std::pair<Record, std::string>> all{};
        for (const auto& record: m_context.repository().list_webhook_events(
                 ::app::store::WebhookEventQuery{}.merchant_id(merchant_id)
             )) {
            const auto payload = payload_of(record, merchant_id);
            if (!is_initial_attempt(record, payload)) {
                continue;
            }
            if (!profile_id.empty()) {
                if (profile_id_of(payload, profile_id) != profile_id) {
                    continue;
                }
            } else if (initiator_of(record, payload) != merchant_id) {
                continue;
            }
            const auto created = parse_timestamp(record.get_created_at());
            if (created && (*created < created_after || *created > created_before)) {
                continue;
            }
            if (!event_types.empty() && !contains(event_types, record.get_event_type())) {
                continue;
            }
            if (internal.is_delivered &&
                delivery_success_of(record, payload, DeliverySource::ListInitialEvents) !=
                    *internal.is_delivered) {
                continue;
            }
            if (internal.recipient &&
                !recipient_matches(recipient_of(payload), *internal.recipient)) {
                continue;
            }
            all.emplace_back(record, payload);
        }

        // created_at DESC ordering (upstream `.order(created_at.desc())`).
        std::stable_sort(
            all.begin(),
            all.end(),
            [](const auto& lhs, const auto& rhs)
            {
                return lhs.first.get_created_at() > rhs.first.get_created_at();
            }
        );

        EventListResult result{};
        // Separate COUNT query: counts every matching row, ignoring limit/offset.
        result.total_count = static_cast<std::int64_t>(all.size());

        // limit/offset applied after ordering.
        std::size_t skipped{0};
        for (const auto& [record, payload]: all) {
            if (offset && skipped++ < static_cast<std::size_t>(*offset)) {
                continue;
            }
            if (result.items.size() >= static_cast<std::size_t>(limit)) {
                break;
            }
            auto item =
                to_list_item(record, payload, DeliverySource::ListInitialEvents, profile_id);
            if (!item) {
                return std::unexpected{std::move(item.error())};
            }
            result.items.push_back(std::move(*item));
        }
        return result;
    }

    [[nodiscard]] std::
        expected<::app::dto::webhook_events::EventListItemResponse, core::error::AppError>
        to_list_item(
            const Record& record,
            std::string_view payload,
            DeliverySource source,
            std::string_view fallback_profile_id
        ) const
    {
        // Upstream requires merchant_id, business_profile_id and initial_attempt_id
        // to be populated (transformers.rs:2285-2300).
        if (record.get_merchant_id().empty()) {
            return std::unexpected{
                error(core::error::AppErrorKind::INTERNAL_ERROR, "missing merchant_id")
            };
        }
        const std::string profile = profile_id_of(payload, fallback_profile_id);
        if (profile.empty()) {
            return std::unexpected{
                error(core::error::AppErrorKind::INTERNAL_ERROR, "missing business_profile_id")
            };
        }

        ::app::dto::webhook_events::EventListItemResponse item;
        item.set_event_id(record.get_event_id());

        ::app::dto::webhook_events::MerchantIdValue merchant;
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(std::move(merchant));

        ::app::dto::webhook_events::ProfileIdValue value;
        value.set_value(profile);
        item.set_profile_id(std::move(value));

        item.set_object_id(object_id_of(payload));

        ::app::dto::webhook_events::EventTypeValue type;
        type.set_value(record.get_event_type());
        item.set_event_type(std::move(type));

        ::app::dto::webhook_events::EventClassValue event_class;
        event_class.set_value(record.get_event_class());
        item.set_event_class(std::move(event_class));

        item.set_is_delivery_successful(delivery_success_of(record, payload, source));

        ::app::dto::webhook_events::PrimitiveDateTimeValue created;
        created.set_value(record.get_created_at());
        item.set_created(std::move(created));

        item.set_initial_attempt_id(initial_attempt_id_of(record, payload));

        const std::string processor = payload_field(payload, "processor_merchant_id");
        if (!processor.empty()) {
            ::app::dto::webhook_events::MerchantIdValue processor_value;
            processor_value.set_value(processor);
            item.set_processor_merchant_id(std::move(processor_value));
        }
        return item;
    }

    // Redact header values exactly like core/webhooks/utils.rs:576-588.
    // nullopt sensitive names => every value is redacted.
    [[nodiscard]] ::app::dto::webhook_events::EventRetrieveResponse to_retrieve(
        const ::app::dto::webhook_events::EventListItemResponse& item,
        const Record& record,
        std::string_view payload,
        const std::optional<std::vector<std::string>>& sensitive
    ) const
    {
        ::app::dto::webhook_events::EventRetrieveResponse response;
        response.set_event_information(item);

        ::app::dto::webhook_events::SecretStringValue body;
        body.set_value(std::string{payload});
        ::app::dto::webhook_events::OutgoingWebhookRequestContent request;
        request.set_body(std::move(body));

        std::vector<std::pair<std::string, ::app::dto::webhook_events::SecretStringValue>>
            headers{};
        for (const auto& [name, value]: parse_headers(payload)) {
            const std::string lowered = to_lower(name);
            const bool redact = !sensitive || contains(*sensitive, lowered);
            ::app::dto::webhook_events::SecretStringValue masked;
            masked.set_value(redact ? std::string{"*** ***"} : std::string{value});
            headers.emplace_back(name, std::move(masked));
        }
        request.set_headers(std::move(headers));
        response.set_request(std::move(request));

        ::app::dto::webhook_events::OutgoingWebhookResponseContent outgoing;
        ::app::dto::webhook_events::SecretStringValue response_body;
        response_body.set_value(record.get_status());
        outgoing.set_body(std::move(response_body));
        response.set_response(std::move(outgoing));

        ::app::dto::webhook_events::WebhookDeliveryAttemptValue attempt;
        const std::string stored_attempt = payload_field(payload, "delivery_attempt");
        attempt.set_value(stored_attempt.empty() ? std::string{"initial_attempt"} : stored_attempt);
        response.set_delivery_attempt(std::move(attempt));
        return response;
    }

    // Parse "headers":[["name","value"], ...] from the persisted request content.
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
    parse_headers(std::string_view payload) const
    {
        std::vector<std::pair<std::string, std::string>> headers{};
        std::size_t cursor = payload.find("\"headers\"");
        if (cursor == std::string_view::npos) {
            return headers;
        }
        cursor += 9;
        while (cursor < payload.size() && payload[cursor] != '[') {
            ++cursor;
        }
        if (cursor >= payload.size()) {
            return headers;
        }
        ++cursor;
        while (cursor < payload.size()) {
            while (cursor < payload.size() &&
                   (std::isspace(static_cast<unsigned char>(payload[cursor])) ||
                    payload[cursor] == ',')) {
                ++cursor;
            }
            if (cursor >= payload.size() || payload[cursor] == ']') {
                break;
            }
            if (payload[cursor] != '[') {
                break;
            }
            ++cursor;
            const auto name = parse_quoted(payload, cursor);
            while (cursor < payload.size() &&
                   (std::isspace(static_cast<unsigned char>(payload[cursor])) ||
                    payload[cursor] == ',')) {
                ++cursor;
            }
            const auto value = parse_quoted(payload, cursor);
            if (!name && !value) {
                break;
            }
            headers.emplace_back(name.value_or(""), value.value_or(""));
            while (cursor < payload.size() && payload[cursor] != ']' && payload[cursor] != ',') {
                ++cursor;
            }
            if (cursor < payload.size() && payload[cursor] == ']') {
                ++cursor;
            }
        }
        return headers;
    }

    [[nodiscard]] std::optional<std::string>
    parse_quoted(std::string_view text, std::size_t& cursor) const
    {
        while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor]))) {
            ++cursor;
        }
        if (cursor >= text.size() || text[cursor] != '"') {
            return std::nullopt;
        }
        ++cursor;
        std::string out{};
        while (cursor < text.size() && text[cursor] != '"') {
            if (text[cursor] == '\\' && cursor + 1 < text.size()) {
                ++cursor;
            }
            out.push_back(text[cursor++]);
        }
        if (cursor < text.size()) {
            ++cursor;
        }
        return out;
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch: value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return out;
    }

    // Local substitute for business_profile.get_sensitive_webhook_header_names().
    // nullopt means the profile could not be resolved: redact every value.
    [[nodiscard]] std::optional<std::vector<std::string>>
    sensitive_header_names(const std::string& profile_id) const
    {
        if (profile_id.empty()) {
            return std::nullopt;
        }
        const auto configured =
            m_context.config_store().get_value("webhook_sensitive_headers", profile_id);
        if (!configured) {
            return std::nullopt;
        }
        std::vector<std::string> names{};
        std::size_t start{0};
        while (start <= configured->size()) {
            const std::size_t comma = configured->find(',', start);
            const std::string_view part = std::string_view{*configured}.substr(
                start,
                comma == std::string_view::npos ? std::string_view::npos : comma - start
            );
            std::size_t first{0};
            std::size_t last{part.size()};
            while (first < last && std::isspace(static_cast<unsigned char>(part[first]))) {
                ++first;
            }
            while (last > first && std::isspace(static_cast<unsigned char>(part[last - 1]))) {
                --last;
            }
            if (last > first) {
                names.emplace_back(to_lower(part.substr(first, last - first)));
            }
            if (comma == std::string_view::npos) {
                break;
            }
            start = comma + 1;
        }
        return names;
    }

    // ---- upstream id generation (core/webhooks/utils.rs:229-261) --------------

    // generate_event_id(): "evt_" + UUIDv7 simple (32 hex digits).
    [[nodiscard]] std::string generate_event_id() const
    {
        std::string raw = m_context.ids().uuid_v4();
        std::string simple{};
        simple.reserve(32);
        for (const char ch: raw) {
            if (ch != '-') {
                simple.push_back(ch);
            }
        }
        if (simple.size() == 32) {
            simple[12] = '7';
        }
        return "evt_" + simple;
    }

    // get_idempotent_event_id(): base64url(SHA256(primary_object_id_event_type))
    // plus an 8-character NanoID suffix for retries.
    [[nodiscard]] std::string get_idempotent_event_id(
        const std::string& primary_object_id,
        const std::string& event_type,
        std::string_view delivery_attempt
    ) const
    {
        const std::string common = primary_object_id + "_" + event_type;
        const auto digest = sha256_bytes(common);
        const std::string encoded = ::app::crypto::Base64Url{}.encode(
            std::string_view{reinterpret_cast<const char*>(digest.data()), digest.size()}
        );
        if (delivery_attempt == "manual_retry" || delivery_attempt == "automatic_retry") {
            return encoded + "_" + nanoid_suffix(8);
        }
        return encoded;
    }

    [[nodiscard]] std::string nanoid_suffix(std::size_t length) const
    {
        static constexpr std::string_view alphabets{
            "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        };
        std::string out{};
        out.reserve(length);
        for (std::size_t i{0}; i < length; ++i) {
            const auto index =
                std::hash<std::string>{}(m_context.ids().uuid_v4()) % alphabets.size();
            out.push_back(alphabets[index]);
        }
        return out;
    }

    [[nodiscard]] std::array<unsigned char, 32> sha256_bytes(std::string_view data) const
    {
        static constexpr std::array<std::uint32_t, 64> k{
            0x42'8A'2F'98U, 0x71'37'44'91U, 0xB5'C0'FB'CFU, 0xE9'B5'DB'A5U, 0x39'56'C2'5BU,
            0x59'F1'11'F1U, 0x92'3F'82'A4U, 0xAB'1C'5E'D5U, 0xD8'07'AA'98U, 0x12'83'5B'01U,
            0x24'31'85'BEU, 0x55'0C'7D'C3U, 0x72'BE'5D'74U, 0x80'DE'B1'FEU, 0x9B'DC'06'A7U,
            0xC1'9B'F1'74U, 0xE4'9B'69'C1U, 0xEF'BE'47'86U, 0x0F'C1'9D'C6U, 0x24'0C'A1'CCU,
            0x2D'E9'2C'6FU, 0x4A'74'84'AAU, 0x5C'B0'A9'DCU, 0x76'F9'88'DAU, 0x98'3E'51'52U,
            0xA8'31'C6'6DU, 0xB0'03'27'C8U, 0xBF'59'7F'C7U, 0xC6'E0'0B'F3U, 0xD5'A7'91'47U,
            0x06'CA'63'51U, 0x14'29'29'67U, 0x27'B7'0A'85U, 0x2E'1B'21'38U, 0x4D'2C'6D'FCU,
            0x53'38'0D'13U, 0x65'0A'73'54U, 0x76'6A'0A'BBU, 0x81'C2'C9'2EU, 0x92'72'2C'85U,
            0xA2'BF'E8'A1U, 0xA8'1A'66'4BU, 0xC2'4B'8B'70U, 0xC7'6C'51'A3U, 0xD1'92'E8'19U,
            0xD6'99'06'24U, 0xF4'0E'35'85U, 0x10'6A'A0'70U, 0x19'A4'C1'16U, 0x1E'37'6C'08U,
            0x27'48'77'4CU, 0x34'B0'BC'B5U, 0x39'1C'0C'B3U, 0x4E'D8'AA'4AU, 0x5B'9C'CA'4FU,
            0x68'2E'6F'F3U, 0x74'8F'82'EEU, 0x78'A5'63'6FU, 0x84'C8'78'14U, 0x8C'C7'02'08U,
            0x90'BE'FF'FAU, 0xA4'50'6C'EBU, 0xBE'F9'A3'F7U, 0xC6'71'78'F2U,
        };

        std::uint32_t h[8]{
            0x6A'09'E6'67U,
            0xBB'67'AE'85U,
            0x3C'6E'F3'72U,
            0xA5'4F'F5'3AU,
            0x51'0E'52'7FU,
            0x9B'05'68'8CU,
            0x1F'83'D9'ABU,
            0x5B'E0'CD'19U,
        };

        std::vector<unsigned char> message{data.begin(), data.end()};
        const std::uint64_t bit_length = static_cast<std::uint64_t>(message.size()) * 8U;
        message.push_back(0x80U);
        while (message.size() % 64U != 56U) {
            message.push_back(0x00U);
        }
        for (int shift{56}; shift >= 0; shift -= 8) {
            message.push_back(static_cast<unsigned char>(bit_length >> shift));
        }

        const auto rotr = [](std::uint32_t value, unsigned bits)
        {
            return (value >> bits) | (value << (32U - bits));
        };

        for (std::size_t offset{0}; offset < message.size(); offset += 64U) {
            std::uint32_t w[64]{};
            for (std::size_t i{0}; i < 16U; ++i) {
                w[i] = (static_cast<std::uint32_t>(message[offset + i * 4]) << 24) |
                       (static_cast<std::uint32_t>(message[offset + i * 4 + 1]) << 16) |
                       (static_cast<std::uint32_t>(message[offset + i * 4 + 2]) << 8) |
                       static_cast<std::uint32_t>(message[offset + i * 4 + 3]);
            }
            for (std::size_t i{16}; i < 64U; ++i) {
                const std::uint32_t s0 =
                    rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            std::uint32_t a{h[0]}, b{h[1]}, c{h[2]}, d{h[3]};
            std::uint32_t e{h[4]}, f{h[5]}, g{h[6]}, hh{h[7]};
            for (std::size_t i{0}; i < 64U; ++i) {
                const std::uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                const std::uint32_t choice = (e & f) ^ (~e & g);
                const std::uint32_t temp1 = hh + s1 + choice + k[i] + w[i];
                const std::uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                const std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
                const std::uint32_t temp2 = s0 + majority;
                hh = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }
            h[0] += a;
            h[1] += b;
            h[2] += c;
            h[3] += d;
            h[4] += e;
            h[5] += f;
            h[6] += g;
            h[7] += hh;
        }

        std::array<unsigned char, 32> digest{};
        for (std::size_t i{0}; i < 8U; ++i) {
            digest[i * 4] = static_cast<unsigned char>(h[i] >> 24);
            digest[i * 4 + 1] = static_cast<unsigned char>(h[i] >> 16);
            digest[i * 4 + 2] = static_cast<unsigned char>(h[i] >> 8);
            digest[i * 4 + 3] = static_cast<unsigned char>(h[i]);
        }
        return digest;
    }

    // Full upstream Event field set for a manual retry (core:397-429).
    [[nodiscard]] std::string build_retry_payload(
        const std::string& new_event_id,
        const std::string& initial_attempt_id,
        const std::string& idempotent_event_id,
        const std::string& processor_merchant_id,
        const std::string& provider_merchant_id,
        const std::string& business_profile_id,
        const std::string& initiator_merchant_id,
        const std::string& recipient,
        std::string_view original_payload
    ) const
    {
        const std::string primary_object_id = object_id_of(original_payload);
        const std::string primary_object_type =
            payload_field(original_payload, "primary_object_type");
        const std::string primary_object_created_at =
            payload_field(original_payload, "primary_object_created_at");
        const std::string metadata = payload_field(original_payload, "metadata");
        return std::format(
            "{{\"event_id\":\"{}\",\"initial_attempt_id\":\"{}\",\"idempotent_event_id\":\"{}\","
            "\"processor_merchant_id\":\"{}\",\"merchant_id\":\"{}\",\"business_profile_id\":\"{}"
            "\","
            "\"primary_object_id\":\"{}\",\"primary_object_type\":\"{}\",\"primary_object_created_"
            "at\":\"{}\","
            "\"initiator_merchant_id\":\"{}\",\"recipient\":\"{}\",\"metadata\":\"{}\","
            "\"delivery_attempt\":\"manual_retry\",\"is_webhook_notified\":false,"
            "\"is_overall_delivery_successful\":false,\"created_at\":\"{}\",\"request\":\"{}\"}}",
            json_escape(new_event_id),
            json_escape(initial_attempt_id),
            json_escape(idempotent_event_id),
            json_escape(processor_merchant_id),
            json_escape(provider_merchant_id),
            json_escape(business_profile_id),
            json_escape(primary_object_id),
            json_escape(primary_object_type),
            json_escape(primary_object_created_at),
            json_escape(initiator_merchant_id),
            json_escape(recipient),
            json_escape(metadata),
            json_escape(now_value()),
            json_escape(original_payload)
        );
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::webhook_events
