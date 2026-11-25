export module app_handlers_payouts:runtime_common;

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
import app_locking;

import :input;

export namespace app::handlers::payouts {

/**
 * @brief Wire values of `common_enums::PayoutStatus` (`enums.rs:9354-9377`,
 * `#[serde(rename_all = "snake_case")]`). Exact upstream values replace the earlier
 * local "pending"/"authorized"/"fulfilled" strings.
 */
inline constexpr std::string_view kRequiresCreation{"requires_creation"};
inline constexpr std::string_view kRequiresConfirmation{"requires_confirmation"};
inline constexpr std::string_view kRequiresPayoutMethodData{"requires_payout_method_data"};
inline constexpr std::string_view kRequiresFulfillment{"requires_fulfillment"};
inline constexpr std::string_view kRequiresVendorAccountCreation{
    "requires_vendor_account_creation"
};
inline constexpr std::string_view kInitiated{"initiated"};
inline constexpr std::string_view kPending{"pending"};
inline constexpr std::string_view kSuccess{"success"};
inline constexpr std::string_view kFailed{"failed"};
inline constexpr std::string_view kCancelled{"cancelled"};
inline constexpr std::string_view kExpired{"expired"};
inline constexpr std::string_view kReversed{"reversed"};
inline constexpr std::string_view kIneligible{"ineligible"};
inline constexpr std::string_view kNotPermitted{"not_permitted"};

/** @brief Per-payout Hold lock, released on every exit path via RAII. */
class PayoutLockGuard
{
public:
    PayoutLockGuard(app::locking::LockRegistry& locks, std::string_view key, std::string owner) :
        m_locks{locks},
        m_key{key}
    {
        m_lock = m_locks.acquire(m_key, std::int64_t{30}, std::move(owner));
    }

    ~PayoutLockGuard()
    {
        if (m_lock.has_value()) {
            m_locks.release(m_key);
        }
    }

    PayoutLockGuard(const PayoutLockGuard&) = delete;
    PayoutLockGuard& operator=(const PayoutLockGuard&) = delete;

    [[nodiscard]] bool held() const noexcept
    {
        return m_lock.has_value();
    }

private:
    app::locking::LockRegistry& m_locks;
    std::string m_key{};
    std::optional<app::locking::ApiLock> m_lock{};
};

struct ConnectorChoice
{
    std::shared_ptr<app::connector::IConnector> connector{};
    std::string decision_action{};
    std::string decision_connector{};
    std::optional<core::error::AppError> error{};
};

struct ConnectorOutcome
{
    std::string status{};
    std::string reference_id{};
    std::string message{};
    bool shadow_killed{};
    std::optional<core::error::AppError> error{};
};

/** @brief Shared business helpers for the payouts sub-runtimes. */
class PayoutsRuntimeSupport
{
public:
    explicit PayoutsRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string out{value};
        for (char& character: out) {
            if (character >= 'A' && character <= 'Z') {
                character = static_cast<char>(character - 'A' + 'a');
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<std::int64_t> to_number(std::string_view value) const
    {
        std::int64_t parsed{0};
        const char* begin = value.data();
        const char* end = value.data() + value.size();
        const auto result = std::from_chars(begin, end, parsed);
        if (result.ec != std::errc{} || result.ptr != end) {
            return std::nullopt;
        }
        return parsed;
    }

    [[nodiscard]] std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char character: value) {
            if (character == '"' || character == '\\') {
                out.push_back('\\');
            }
            out.push_back(character == '\n' ? ' ' : character);
        }
        return out;
    }

    [[nodiscard]] std::string meta_value(std::string_view json, std::string_view key) const
    {
        const std::string needle = std::format("\"{}\":\"", key);
        const auto at = json.find(needle);
        if (at == std::string_view::npos) {
            return {};
        }
        const auto start = at + needle.size();
        const auto end = json.find('"', start);
        if (end == std::string_view::npos) {
            return {};
        }
        return std::string{json.substr(start, end - start)};
    }

    [[nodiscard]] std::string encode_meta(
        std::string_view profile_id,
        std::string_view browser_info,
        std::string_view payout_link_id,
        std::string_view payout_link_status,
        std::string_view note,
        std::string_view merchant_order_reference_id = {},
        std::string_view entity_type = {}
    ) const
    {
        return std::format(
            R"({{"profile_id":"{}","browser_info":"{}","payout_link_id":"{}","payout_link_status":"{}","note":"{}","merchant_order_reference_id":"{}","entity_type":"{}"}})",
            json_escape(profile_id),
            json_escape(browser_info),
            json_escape(payout_link_id),
            json_escape(payout_link_status),
            json_escape(note),
            json_escape(merchant_order_reference_id),
            json_escape(entity_type)
        );
    }

    [[nodiscard]] std::string
    merge_meta_profile(std::string_view meta, std::string_view profile_id) const
    {
        return encode_meta(
            profile_id,
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            meta_value(meta, "payout_link_status"),
            meta_value(meta, "note"),
            meta_value(meta, "merchant_order_reference_id"),
            meta_value(meta, "entity_type")
        );
    }

    [[nodiscard]] std::string
    merge_meta_browser_info(std::string_view meta, std::string_view browser_info) const
    {
        return encode_meta(
            meta_value(meta, "profile_id"),
            browser_info,
            meta_value(meta, "payout_link_id"),
            meta_value(meta, "payout_link_status"),
            meta_value(meta, "note"),
            meta_value(meta, "merchant_order_reference_id"),
            meta_value(meta, "entity_type")
        );
    }

    [[nodiscard]] std::string
    merge_meta_payout_link(std::string_view meta, std::string_view link_status) const
    {
        return encode_meta(
            meta_value(meta, "profile_id"),
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            link_status,
            meta_value(meta, "note"),
            meta_value(meta, "merchant_order_reference_id"),
            meta_value(meta, "entity_type")
        );
    }

    [[nodiscard]] std::string merge_meta_note(std::string_view meta, std::string_view note) const
    {
        return encode_meta(
            meta_value(meta, "profile_id"),
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            meta_value(meta, "payout_link_status"),
            note,
            meta_value(meta, "merchant_order_reference_id"),
            meta_value(meta, "entity_type")
        );
    }

    [[nodiscard]] std::string first_requested_connector(
        const std::optional<std::vector<app::model::payouts::PayoutConnectorsValue>>& connectors
    ) const
    {
        if (connectors.has_value() && !connectors->empty()) {
            return connectors->front().get_value();
        }
        return {};
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_create_input(
        const app::model::payouts::PayoutCreateRequest& input,
        std::string_view header_merchant_id
    ) const
    {
        if (!input.amount.has_value()) {
            return make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "payout amount is required"
            );
        }
        if (!input.currency.has_value() || input.currency->get_value().empty()) {
            return make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "payout currency is required"
            );
        }
        const bool has_method =
            input.payout_method_data.has_value() || input.payout_method_id.has_value() ||
            input.payout_type.has_value() ||
            (input.payout_token.has_value() && !input.payout_token->empty()) ||
            input.payout_link.value_or(false) || input.payout_link_config.has_value();
        if (!has_method) {
            return make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "a payout method is required"
            );
        }
        if (input.merchant_id.has_value() && !input.merchant_id->get_value().empty() &&
            !header_merchant_id.empty() && input.merchant_id->get_value() != header_merchant_id) {
            return make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "payout merchant does not match the request merchant"
            );
        }
        return std::nullopt;
    }

    void store_payout_method_data(std::string_view payout_id, std::string_view raw_payload) const
    {
        if (raw_payload.empty()) {
            return;
        }
        m_context.key_store().put(
            "payout_method_data",
            payout_id,
            "raw",
            app::crypto::SecretString{std::string{raw_payload}}
        );
        m_context.metrics().counter("payouts_method_data_stored").increment();
    }

    [[nodiscard]] ConnectorChoice make_connector_decision(
        app::connector::ConnectorRequest& request,
        std::string_view merchant_id,
        std::string_view payout_id,
        std::int64_t amount,
        std::string_view currency,
        std::string_view payout_type,
        std::string_view requested_connector
    ) const
    {
        // Local implementation of `get_connector_choice` + `make_connector_decision`
        // (`core/payouts.rs:167-418`, `:586-623`). UCS/routing tiers are external infra;
        // the local `ConnectorRouter` substitutes for them. No config-derived decision
        // program is consulted — the eligible connector comes from the request's `connector`
        // field (`payouts.rs:81` -> `core/payouts.rs:758`) and router ranking.
        ConnectorChoice choice{};
        (void)merchant_id;
        (void)payout_id;
        (void)amount;
        (void)currency;
        request.set_payment_method_type(std::string{payout_type});
        auto connector = m_context.connector_router().select_for(request);
        if (!connector) {
            const auto last = m_context.connector_router().last_error();
            if (last.has_value() && last->get_message() != "no connector registered") {
                choice.error = *last;
            } else {
                m_context.metrics().counter("payouts_connector_unavailable").increment();
            }
            return choice;
        }
        if (!requested_connector.empty() && connector->name() != requested_connector) {
            // Upstream restricts the decision to the merchant-supplied eligible connector
            // list (`PayoutCreateRequest.connector`, `core/payouts.rs:758`).
            m_context.metrics().counter("payouts_connector_unavailable").increment();
            return choice;
        }
        choice.decision_action = "allow";
        choice.decision_connector = std::string{connector->name()};
        choice.connector = std::move(connector);
        m_context.metrics().record_flow("payout.connector.decision");
        m_context.metrics().counter("payouts_connector_decision").increment();
        return choice;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    map_connector_error(const app::connector::ConnectorResponse& response) const
    {
        const auto combined =
            to_lower(std::format("{} {}", response.get_status(), response.get_message()));
        if (combined.find("timeout") != std::string::npos ||
            combined.find("timed_out") != std::string::npos) {
            return make_error(core::error::AppErrorKind::TIMEOUT, "connector timed out");
        }
        if (combined.find("5xx") != std::string::npos ||
            combined.find("server_error") != std::string::npos) {
            return make_error(core::error::AppErrorKind::CONNECTOR_ERROR, "connector server error");
        }
        if (response.get_status().empty() || combined.find("failed") != std::string::npos ||
            combined.find("declined") != std::string::npos ||
            combined.find("error") != std::string::npos) {
            return make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                response.get_message().empty() ? std::string{"connector payout failed"}
                                               : response.get_message()
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] ConnectorOutcome invoke_connector(
        app::connector::IConnector& connector,
        std::string_view verb,
        const app::connector::ConnectorRequest& request
    ) const
    {
        ConnectorOutcome outcome{};
        const auto response = [&connector, verb, &request]()
        {
            if (verb == "cancel") {
                return connector.cancel(request);
            }
            if (verb == "capture") {
                return connector.capture(request);
            }
            if (verb == "sync") {
                return connector.sync(request);
            }
            return connector.authorize(request);
        }();
        outcome.status = response.get_status();
        outcome.reference_id = response.get_reference_id();
        outcome.message = response.get_message();
        outcome.shadow_killed = response.get_shadow_killed();
        outcome.error = map_connector_error(response);
        return outcome;
    }

    /** @brief Maps a local connector outcome to an upstream `PayoutStatus` wire value. */
    [[nodiscard]] std::string
    status_from_connector(std::string_view status, bool auto_fulfill) const
    {
        const auto lower = to_lower(status);
        if (lower == "authorized" || lower == "requires_fulfillment") {
            return auto_fulfill ? std::string{kSuccess} : std::string{kRequiresFulfillment};
        }
        if (lower.empty()) {
            return std::string{kRequiresFulfillment};
        }
        return normalize_status(status);
    }

    /** @brief Normalizes legacy local status strings to upstream snake_case values. */
    [[nodiscard]] std::string normalize_status(std::string_view status) const
    {
        const auto lower = to_lower(status);
        if (lower == "succeeded") {
            return std::string{kSuccess};
        }
        if (lower == "authorized") {
            return std::string{kRequiresFulfillment};
        }
        if (lower == "fulfilled") {
            return std::string{kSuccess};
        }
        if (lower == "notpermitted") {
            return std::string{kNotPermitted};
        }
        if (lower == "requiresfulfillment") {
            return std::string{kRequiresFulfillment};
        }
        if (lower == "requirescreation") {
            return std::string{kRequiresCreation};
        }
        if (lower == "requiresconfirmation") {
            return std::string{kRequiresConfirmation};
        }
        if (lower == "requirespayoutmethoddata") {
            return std::string{kRequiresPayoutMethodData};
        }
        if (lower == "requiresvendoraccountcreation") {
            return std::string{kRequiresVendorAccountCreation};
        }
        return std::string{lower};
    }

    /**
     * @brief Exact port of `Connector::supports_instant_payout` (`connector_enums.rs:239-258`).
     * A missing connector name maps to `true` (`core/payouts.rs:1009-1012` `unwrap_or(true)`).
     */
    [[nodiscard]] bool connector_supports_instant_payout(
        std::string_view connector_name,
        std::string_view payout_type
    ) const
    {
        const auto connector = to_lower(connector_name);
        const auto method = to_lower(payout_type);
        if (connector == "paypal") {
            return method == "wallet";
        }
        if (connector == "checkout" || connector == "envoy" || connector == "adyenplatform" ||
            connector == "nomupay" || connector == "loonio" || connector == "truelayer" ||
            connector == "trustly" || connector == "gotymesanlam") {
            return true;
        }
        if (method == "card") {
            return true;
        }
        if (connector == "worldpay" || connector == "worldpayxml") {
            return method == "wallet";
        }
        if (connector == "itaubank" || connector == "deutschebank") {
            return method == "bank";
        }
        if (connector == "mifinity") {
            return method == "wallet" || method == "bank";
        }
        return false;
    }

    [[nodiscard]] bool
    supports_instant_payout(std::string_view connector_name, std::string_view payout_type) const
    {
        if (connector_name.empty()) {
            return true;
        }
        return connector_supports_instant_payout(connector_name, payout_type);
    }

    /** @brief `PayoutStatus::is_terminal_status` (`enums.rs:9391-9401`). */
    [[nodiscard]] bool is_payout_terminal_state(std::string_view status) const
    {
        const auto value = normalize_status(status);
        return value != kRequiresCreation && value != kRequiresConfirmation &&
               value != kRequiresPayoutMethodData && value != kRequiresVendorAccountCreation &&
               value != kPending && value != kInitiated && value != kRequiresFulfillment;
    }

    /** @brief `helpers::is_payout_initiated` (`core/payouts/helpers.rs:1350-1359`). */
    [[nodiscard]] bool is_payout_initiated(std::string_view status) const
    {
        const auto value = normalize_status(status);
        return value != kRequiresCreation && value != kRequiresConfirmation &&
               value != kRequiresPayoutMethodData && value != kRequiresVendorAccountCreation &&
               value != kInitiated;
    }

    /** @brief `helpers::is_eligible_for_local_payout_cancellation` (`helpers.rs:1406-1415`). */
    [[nodiscard]] bool is_eligible_for_local_payout_cancellation(std::string_view status) const
    {
        const auto value = normalize_status(status);
        return value == kRequiresCreation || value == kRequiresConfirmation ||
               value == kRequiresPayoutMethodData || value == kRequiresVendorAccountCreation ||
               value == kRequiresFulfillment;
    }

    /** @brief `helpers::should_call_retrieve` (`helpers.rs:1389-1394`). */
    [[nodiscard]] bool should_call_retrieve(std::string_view status) const
    {
        const auto value = normalize_status(status);
        return value == kPending || value == kInitiated;
    }

    /** @brief `payouts_confirm_core` disallowed statuses (`core/payouts.rs:791-804`). */
    [[nodiscard]] bool is_confirm_disallowed(std::string_view status) const
    {
        const auto value = normalize_status(status);
        const std::array<std::string_view, 8> disallowed{
            kCancelled,
            kSuccess,
            kFailed,
            kPending,
            kIneligible,
            kNotPermitted,
            kRequiresFulfillment,
            kRequiresVendorAccountCreation
        };
        return std::find(disallowed.begin(), disallowed.end(), value) != disallowed.end();
    }

    /** @brief `payouts_cancel_core` terminal guard (`core/payouts.rs:999-1006`). */
    [[nodiscard]] bool is_cancel_terminal(std::string_view status) const
    {
        return is_payout_terminal_state(status);
    }

    /** @brief `payouts_fulfill_core` guard (`core/payouts.rs:1111-1120`). */
    [[nodiscard]] bool is_fulfill_allowed(std::string_view status) const
    {
        return !is_payout_terminal_state(status) &&
               normalize_status(status) == kRequiresFulfillment;
    }

    [[nodiscard]] std::optional<std::string>
    json_string_any(std::string_view body, std::initializer_list<std::string_view> keys) const
    {
        for (const auto key: keys) {
            if (auto value = meta_value(body, key); !value.empty()) {
                return value;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] TimeRange time_range_from_body(std::string_view body) const
    {
        TimeRange range{};
        const auto start = json_string_any(body, {"start_time", "startTime"});
        const auto end = json_string_any(body, {"end_time", "endTime"});
        if (!start.has_value() && !end.has_value()) {
            return range;
        }
        range.present = true;
        range.start_time = start.value_or("");
        range.end_time = end;
        return range;
    }

    /** @brief All upstream `PayoutStatus::iter()` values in declaration order
     * (`enums.rs:9354-9377`). */
    [[nodiscard]] std::vector<std::string_view> all_payout_statuses() const
    {
        return {
            kSuccess,
            kFailed,
            kCancelled,
            kInitiated,
            kExpired,
            kReversed,
            kPending,
            kIneligible,
            kNotPermitted,
            kRequiresCreation,
            kRequiresConfirmation,
            kRequiresPayoutMethodData,
            kRequiresFulfillment,
            kRequiresVendorAccountCreation,
        };
    }

    /** @brief ISO8601 UTC timestamp matching `common_utils::date_time::now()` wire format. */
    [[nodiscard]] std::string created_at_now() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
        )
                                 .count();
        const std::chrono::sys_seconds point{std::chrono::seconds{seconds}};
        return std::format("{:%FT%TZ}", point);
    }

    /** @brief `utils::get_payout_attempt_id(payout_id, count)` (`utils.rs:175-178`). */
    [[nodiscard]] std::string payout_attempt_id(std::string_view payout_id, int count = 1) const
    {
        return std::format("{}_{}", payout_id, count);
    }

    [[nodiscard]] bool within_time_range(std::string_view created_at, const TimeRange& range) const
    {
        if (!range.present) {
            return true;
        }
        const auto compare = [this](std::string_view lhs, std::string_view rhs) -> int
        {
            if (const auto l = to_number(lhs); l.has_value()) {
                if (const auto r = to_number(rhs); r.has_value()) {
                    return *l < *r ? -1 : (*l > *r ? 1 : 0);
                }
            }
            if (lhs < rhs) {
                return -1;
            }
            if (lhs > rhs) {
                return 1;
            }
            return 0;
        };
        if (!range.start_time.empty() && compare(created_at, range.start_time) < 0) {
            return false;
        }
        if (range.end_time.has_value() && !range.end_time->empty() &&
            compare(created_at, *range.end_time) > 0) {
            return false;
        }
        return true;
    }

    /**
     * @brief Local `filter_by_constraints` + `filter_objects_based_on_profile_id_list`
     * (`core/payouts.rs:1202-1212`, `:1456-1473`). Empty profile id means upstream
     * `None` (merchant scope); a non-empty id is the single authenticated profile.
     */
    [[nodiscard]] std::vector<app::store::payouts::PayoutRecord>
    scoped_records(std::string_view profile_id, const TimeRange& range) const
    {
        std::vector<app::store::payouts::PayoutRecord> out{};
        for (const auto& record: m_context.repository().list_payouts()) {
            if (!profile_id.empty() &&
                meta_value(record.get_metadata(), "profile_id") != profile_id) {
                continue;
            }
            if (!within_time_range(record.get_created_at(), range)) {
                continue;
            }
            out.push_back(record);
        }
        return out;
    }

    app::store::payout_attempts::PayoutAttemptRecord record_payout_attempt(
        std::string_view payout_id,
        std::string_view merchant_id,
        std::string_view connector,
        std::string_view status,
        std::int64_t amount,
        std::string_view currency,
        std::string_view error_code = {},
        std::string_view error_message = {}
    ) const
    {
        app::store::payout_attempts::PayoutAttemptRecord attempt{};
        attempt.set_attempt_id(payout_attempt_id(payout_id));
        attempt.set_payout_id(std::string{payout_id});
        attempt.set_merchant_id(std::string{merchant_id});
        attempt.set_connector(std::string{connector});
        attempt.set_status(std::string{status});
        attempt.set_amount(amount);
        attempt.set_currency(std::string{currency});
        attempt.set_error_code(std::string{error_code});
        attempt.set_error_message(std::string{error_message});
        return m_context.repository().save_payout_attempt(attempt);
    }

    void update_latest_attempt(
        std::string_view payout_id,
        std::string_view status,
        std::string_view error_code,
        std::string_view error_message
    ) const
    {
        for (auto attempt: m_context.repository().list_payout_attempts()) {
            if (attempt.get_payout_id() != payout_id) {
                continue;
            }
            attempt.set_status(std::string{status});
            if (!error_code.empty()) {
                attempt.set_error_code(std::string{error_code});
            }
            if (!error_message.empty()) {
                attempt.set_error_message(std::string{error_message});
            }
            m_context.repository().update_payout_attempt(attempt);
            return;
        }
    }

    [[nodiscard]] std::vector<app::model::payouts::PayoutAttemptResponse>
    attempts_for(std::string_view payout_id) const
    {
        std::vector<app::model::payouts::PayoutAttemptResponse> out{};
        for (const auto& attempt: m_context.repository().list_payout_attempts()) {
            if (attempt.get_payout_id() != payout_id) {
                continue;
            }
            app::model::payouts::PayoutAttemptResponse item{};
            item.set_attempt_id(attempt.get_attempt_id());
            app::model::payouts::PayoutStatusValue status{};
            status.set_value(attempt.get_status());
            item.set_status(status);
            app::model::payouts::MinorUnitValue amount{};
            amount.set_value(std::to_string(attempt.get_amount()));
            item.set_amount(amount);
            app::model::payouts::CurrencyValue currency{};
            currency.set_value(attempt.get_currency());
            item.set_currency(currency);
            if (!attempt.get_connector().empty()) {
                item.set_connector(attempt.get_connector());
            }
            if (!attempt.get_error_code().empty()) {
                item.set_error_code(attempt.get_error_code());
            }
            if (!attempt.get_error_message().empty()) {
                item.set_error_message(attempt.get_error_message());
            }
            out.push_back(std::move(item));
        }
        return out;
    }

    /**
     * @brief Upstream outgoing webhook trigger (`utils.rs:1470-1537`). The event type is
     * derived from the final PayoutStatus (`common_enums/transformers.rs:2194-2215`); statuses
     * without an event mapping emit nothing. The local EventBus + WebhookDeliveryService
     * substitute for `create_event_and_trigger_outgoing_webhook`.
     */
    void emit_payout_webhook(
        std::string_view,
        const app::store::payouts::PayoutRecord& record
    ) const
    {
        const auto status = normalize_status(record.get_status());
        std::string_view event_type{};
        if (status == kSuccess) {
            event_type = "payout_success";
        } else if (status == kFailed) {
            event_type = "payout_failed";
        } else if (status == kCancelled) {
            event_type = "payout_cancelled";
        } else if (status == kInitiated) {
            event_type = "payout_initiated";
        } else if (status == kExpired) {
            event_type = "payout_expired";
        } else if (status == kReversed) {
            event_type = "payout_reversed";
        } else if (status == kNotPermitted) {
            event_type = "payout_not_permitted";
        }
        if (event_type.empty()) {
            return;
        }
        const auto payload = std::format(
            R"({{"payout_id":"{}","merchant_id":"{}","status":"{}","event_type":"{}","created":"{}","amount":"{}","currency":"{}"}})",
            json_escape(record.get_payout_id()),
            json_escape(record.get_merchant_id()),
            json_escape(status),
            event_type,
            json_escape(record.get_created_at()),
            record.get_amount(),
            json_escape(record.get_currency())
        );
        const auto event_id =
            m_context.events().publish(std::format("payout.{}", event_type), payload);
        m_context.webhook_delivery().enqueue(
            event_id,
            std::format("merchant://{}/webhooks", record.get_merchant_id())
        );
        m_context.metrics().record_flow("payout.webhook");
        m_context.metrics().counter("payouts_webhook_emitted").increment();
    }

    [[nodiscard]] std::optional<app::store::payouts::PayoutRecord>
    find_payout_record(std::string_view payout_id) const
    {
        if (payout_id.empty()) {
            return std::nullopt;
        }
        return m_context.repository().find_payout_by_string(payout_id);
    }

    [[nodiscard]] app::model::payouts::PayoutCreateResponse
    response_from_record(const app::store::payouts::PayoutRecord& record) const
    {
        using Response = app::model::payouts::PayoutCreateResponse;
        Response response{};
        app::model::payouts::PayoutIdValue payout_value{};
        payout_value.set_value(record.get_payout_id());
        response.set_payout_id(payout_value);
        app::model::payouts::MerchantIdValue merchant_value{};
        merchant_value.set_value(record.get_merchant_id());
        response.set_merchant_id(merchant_value);
        app::model::payouts::MinorUnitValue amount_value{};
        amount_value.set_value(std::to_string(record.get_amount()));
        response.set_amount(amount_value);
        app::model::payouts::CurrencyValue currency_value{};
        currency_value.set_value(record.get_currency());
        response.set_currency(currency_value);
        app::model::payouts::PayoutStatusValue status_value{};
        status_value.set_value(record.get_status());
        response.set_status(status_value);
        if (!record.get_connector().empty()) {
            response.set_connector(record.get_connector());
        }
        if (!record.get_payout_type().empty()) {
            app::model::payouts::PayoutTypeValue type_value{};
            type_value.set_value(record.get_payout_type());
            response.set_payout_type(type_value);
        }
        if (!record.get_customer_id().empty()) {
            app::model::payouts::CustomerIdValue customer_value{};
            customer_value.set_value(record.get_customer_id());
            response.set_customer_id(customer_value);
        }
        return response;
    }

    void enrich_response(
        app::model::payouts::PayoutCreateResponse& response,
        const app::store::payouts::PayoutRecord& record,
        bool expand_attempts
    ) const
    {
        const auto metadata = record.get_metadata();
        const auto profile_id = meta_value(metadata, "profile_id");
        if (!profile_id.empty()) {
            app::model::payouts::ProfileIdValue profile{};
            profile.set_value(profile_id);
            response.set_profile_id(profile);
        }
        if (!record.get_created_at().empty()) {
            app::model::payouts::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            response.set_created(created);
        }
        const auto payout_link_id = meta_value(metadata, "payout_link_id");
        if (!payout_link_id.empty()) {
            app::model::payouts::PayoutLinkResponse link{};
            link.set_payout_link_id(payout_link_id);
            app::model::payouts::SecretValue secret{};
            secret.set_value(std::format("/payout-links/{}", payout_link_id));
            link.set_link(secret);
            response.set_payout_link(link);
        }
        if (expand_attempts) {
            response.set_attempts(attempts_for(record.get_payout_id()));
        }
    }

    [[nodiscard]] std::vector<std::string> distinct_field(
        const std::vector<app::store::payouts::PayoutRecord>& records,
        const std::function<std::string(const app::store::payouts::PayoutRecord&)>& getter
    ) const
    {
        std::vector<std::string> values{};
        for (const auto& record: records) {
            const auto value = getter(record);
            if (value.empty()) {
                continue;
            }
            if (std::find(values.begin(), values.end(), value) == values.end()) {
                values.emplace_back(value);
            }
        }
        return values;
    }

    /** @brief `get_aggregates_for_payouts` (`core/payouts.rs:4502-4529`): all statuses,
     * zero-filled. */
    [[nodiscard]] app::model::payouts::PayoutsAggregateResponse
    aggregate_response(const std::vector<app::store::payouts::PayoutRecord>& records) const
    {
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto status: all_payout_statuses()) {
            counts.emplace(std::string{status}, 0);
        }
        for (const auto& record: records) {
            ++counts[normalize_status(record.get_status())];
        }
        app::model::payouts::PayoutsAggregateResponse response{};
        response.set_status_with_count(std::move(counts));
        return response;
    }

    /**
     * @brief Local `get_filters_for_payouts` (`core/payouts.rs:1475-1491`).
     */
    [[nodiscard]] app::model::payouts::PayoutListFilters
    available_filters(const std::vector<app::store::payouts::PayoutRecord>& records) const
    {
        app::model::payouts::PayoutListFilters filters{};
        std::vector<app::model::payouts::PayoutConnectorsValue> connectors{};
        for (const auto& connector: distinct_field(
                 records,
                 [](const auto& r)
                 {
                     return r.get_connector();
                 }
             )) {
            app::model::payouts::PayoutConnectorsValue value{};
            value.set_value(connector);
            connectors.push_back(std::move(value));
        }
        filters.set_connector(std::move(connectors));
        std::vector<app::model::payouts::CurrencyValue> currencies{};
        for (const auto& currency: distinct_field(
                 records,
                 [](const auto& r)
                 {
                     return r.get_currency();
                 }
             )) {
            app::model::payouts::CurrencyValue value{};
            value.set_value(currency);
            currencies.push_back(std::move(value));
        }
        filters.set_currency(std::move(currencies));
        std::vector<app::model::payouts::PayoutStatusValue> statuses{};
        for (const auto& status: distinct_field(
                 records,
                 [](const auto& r)
                 {
                     return r.get_status();
                 }
             )) {
            app::model::payouts::PayoutStatusValue value{};
            value.set_value(normalize_status(status));
            statuses.push_back(std::move(value));
        }
        filters.set_status(std::move(statuses));
        std::vector<app::model::payouts::PayoutTypeValue> methods{};
        for (const auto& method: distinct_field(
                 records,
                 [](const auto& r)
                 {
                     return r.get_payout_type();
                 }
             )) {
            app::model::payouts::PayoutTypeValue value{};
            value.set_value(method);
            methods.push_back(std::move(value));
        }
        filters.set_payout_method(std::move(methods));
        return filters;
    }

    /** @brief `TimeRange.start_time` is mandatory for filter/aggregate routes (`types.rs:814-821`).
     */
    [[nodiscard]] std::optional<core::error::AppError>
    require_time_range(const TimeRange& range) const
    {
        if (!range.present || range.start_time.empty()) {
            return make_error(core::error::AppErrorKind::INVALID_REQUEST, "start_time is required");
        }
        return std::nullopt;
    }

    /** @brief `api_enums::Currency::iter()` (`common_enums/src/enums.rs:1093-1252`). */
    [[nodiscard]] std::vector<std::string_view> all_currencies() const
    {
        static const std::vector<std::string_view> currencies{
            "AED", "AFN", "ALL", "AMD", "ANG", "AOA", "ARS", "AUD", "AWG", "AZN", "BAM", "BBD",
            "BDT", "BGN", "BHD", "BIF", "BMD", "BND", "BOB", "BRL", "BSD", "BTN", "BWP", "BYN",
            "BZD", "CAD", "CDF", "CHF", "CLF", "CLP", "CNY", "COP", "CRC", "CUC", "CUP", "CVE",
            "CZK", "DJF", "DKK", "DOP", "DZD", "EGP", "ERN", "ETB", "EUR", "FJD", "FKP", "GBP",
            "GEL", "GHS", "GIP", "GMD", "GNF", "GTQ", "GYD", "HKD", "HNL", "HRK", "HTG", "HUF",
            "IDR", "ILS", "INR", "IQD", "IRR", "ISK", "JMD", "JOD", "JPY", "KES", "KGS", "KHR",
            "KMF", "KPW", "KRW", "KWD", "KYD", "KZT", "LAK", "LBP", "LKR", "LRD", "LSL", "LYD",
            "MAD", "MDL", "MGA", "MKD", "MMK", "MNT", "MOP", "MRU", "MUR", "MVR", "MWK", "MXN",
            "MYR", "MZN", "NAD", "NGN", "NIO", "NOK", "NPR", "NZD", "OMR", "PAB", "PEN", "PGK",
            "PHP", "PKR", "PLN", "PYG", "QAR", "RON", "RSD", "RUB", "RWF", "SAR", "SBD", "SCR",
            "SDG", "SEK", "SGD", "SHP", "SLE", "SLL", "SOS", "SRD", "SSP", "STD", "STN", "SVC",
            "SYP", "SZL", "THB", "TJS", "TMT", "TND", "TOP", "TRY", "TTD", "TWD", "TZS", "UAH",
            "UGX", "USD", "UYU", "UZS", "VES", "VND", "VUV", "WST", "XAF", "XCD", "XOF", "XPF",
            "YER", "ZAR", "ZMW", "ZWL",
        };
        return currencies;
    }

    [[nodiscard]] std::vector<std::string>
    all_json_string_values(std::string_view json, std::string_view key) const
    {
        const std::string needle = std::format("\"{}\":\"", key);
        std::vector<std::string> out{};
        std::size_t cursor{0};
        while ((cursor = json.find(needle, cursor)) != std::string_view::npos) {
            const auto start = cursor + needle.size();
            const auto end = json.find('"', start);
            if (end == std::string_view::npos) {
                break;
            }
            out.emplace_back(json.substr(start, end - start));
            cursor = end + 1;
        }
        return out;
    }

    /**
     * @brief Exact `get_payout_filters_core` (`core/payouts.rs:1495-1559`): payout
     * merchant connector accounts feed the connector map and payout-method set; currency
     * and status are the full upstream enums. Connector map values are structured
     * `MerchantConnectorInfo` objects matching `api_models/src/admin.rs:1626-1633`.
     */
    [[nodiscard]] app::model::payouts::PayoutListFiltersV2
    filters_v2_from_mcas(std::string_view profile_id, std::string_view merchant_id) const
    {
        app::model::payouts::PayoutListFiltersV2 filters{};
        std::vector<app::model::payouts::CurrencyValue> currencies{};
        for (const auto currency: all_currencies()) {
            app::model::payouts::CurrencyValue value{};
            value.set_value(std::string{currency});
            currencies.push_back(std::move(value));
        }
        filters.set_currency(std::move(currencies));
        std::vector<app::model::payouts::PayoutStatusValue> statuses{};
        for (const auto status: all_payout_statuses()) {
            app::model::payouts::PayoutStatusValue value{};
            value.set_value(std::string{status});
            statuses.push_back(std::move(value));
        }
        filters.set_status(std::move(statuses));

        std::unordered_map<std::string, std::vector<app::model::payouts::MerchantConnectorInfo>>
            connector_map{};
        std::vector<app::model::payouts::PayoutTypeValue> payout_methods{};
        for (const auto& mca: m_context.repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && mca.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!profile_id.empty() && mca.get_profile_id() != profile_id) {
                continue;
            }
            if (mca.get_connector_type() != "payout_processor") {
                continue;
            }
            if (!mca.get_connector_label().empty()) {
                // Upstream `MerchantConnectorResponse::to_merchant_connector_info`
                // (`api_models/src/admin.rs:1726-1732`): connector_label from the label,
                // merchant_connector_id from the MCA id, connector_type from the record.
                app::model::payouts::MerchantConnectorInfo info{};
                info.set_connector_label(mca.get_connector_label());
                info.set_merchant_connector_id(mca.get_merchant_connector_id());
                info.set_connector_type(mca.get_connector_type());
                connector_map[mca.get_connector_name()].push_back(std::move(info));
            }
            for (const auto& method:
                 all_json_string_values(mca.get_payment_methods_enabled(), "payment_method")) {
                if (std::find_if(
                        payout_methods.begin(),
                        payout_methods.end(),
                        [&method](const auto& value)
                        {
                            return value.get_value() == method;
                        }
                    ) == payout_methods.end()) {
                    app::model::payouts::PayoutTypeValue value{};
                    value.set_value(method);
                    payout_methods.push_back(std::move(value));
                }
            }
        }
        filters.set_connector(std::move(connector_map));
        filters.set_payout_method(std::move(payout_methods));
        return filters;
    }

    /**
     * @brief Local `payouts_filtered_list_core` row filtering (`core/payouts.rs:1338-1453`).
     * Every field of the typed `PayoutListFilterConstraints` is applied to persisted rows;
     * `total_count` matches the un-paginated filtered set.
     */
    [[nodiscard]] app::model::payouts::PayoutListResponse list_response(
        const ListFilters& filters,
        std::string_view profile_id,
        const TimeRange& range
    ) const
    {
        const auto records = scoped_records(profile_id, range);
        std::vector<app::store::payouts::PayoutRecord> matched{};
        for (const auto& record: records) {
            if (!filters.payout_id.empty() && record.get_payout_id() != filters.payout_id) {
                continue;
            }
            if (!filters.merchant_order_reference_id.empty() &&
                meta_value(record.get_metadata(), "merchant_order_reference_id") !=
                    filters.merchant_order_reference_id) {
                continue;
            }
            if (!filters.customer_id.empty() && record.get_customer_id() != filters.customer_id) {
                continue;
            }
            if (!filters.connector.empty() && std::find(
                                                  filters.connector.begin(),
                                                  filters.connector.end(),
                                                  record.get_connector()
                                              ) == filters.connector.end()) {
                continue;
            }
            if (!filters.currency.empty() && std::find(
                                                 filters.currency.begin(),
                                                 filters.currency.end(),
                                                 record.get_currency()
                                             ) == filters.currency.end()) {
                continue;
            }
            if (!filters.status.empty()) {
                const auto status = normalize_status(record.get_status());
                const bool any = std::any_of(
                    filters.status.begin(),
                    filters.status.end(),
                    [&status](const auto& c)
                    {
                        return status == c;
                    }
                );
                if (!any) {
                    continue;
                }
            }
            if (!filters.payout_method.empty() && std::find(
                                                      filters.payout_method.begin(),
                                                      filters.payout_method.end(),
                                                      record.get_payout_type()
                                                  ) == filters.payout_method.end()) {
                continue;
            }
            if (!filters.entity_type.empty() &&
                meta_value(record.get_metadata(), "entity_type") != filters.entity_type) {
                continue;
            }
            matched.push_back(record);
        }
        std::vector<app::model::payouts::PayoutCreateResponse> data{};
        for (std::size_t i = filters.offset; i < matched.size(); ++i) {
            if (filters.limit > 0 && data.size() >= filters.limit) {
                break;
            }
            auto item = response_from_record(matched[i]);
            enrich_response(item, matched[i], false);
            data.push_back(std::move(item));
        }
        app::model::payouts::PayoutListResponse response{};
        response.set_data(std::move(data));
        response.set_size(response.get_data().size());
        response.set_total_count(static_cast<std::int64_t>(matched.size()));
        return response;
    }

    [[nodiscard]] std::size_t
    to_count(std::string_view value, const std::optional<std::string>& fallback) const
    {
        std::size_t parsed{0};
        if (!value.empty()) {
            const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (result.ec == std::errc{}) {
                return parsed;
            }
        }
        if (fallback.has_value()) {
            const auto& text = *fallback;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), parsed);
            if (result.ec == std::errc{}) {
                return parsed;
            }
        }
        return 0;
    }

    [[nodiscard]] std::vector<std::string> status_values(
        const std::optional<std::vector<app::model::payouts::PayoutStatusValue>>& values
    ) const
    {
        std::vector<std::string> out{};
        if (values.has_value()) {
            for (const auto& value: *values) {
                out.push_back(normalize_status(value.get_value()));
            }
        }
        return out;
    }

    template<typename Value>
    [[nodiscard]] std::vector<std::string>
    wrapper_values(const std::optional<std::vector<Value>>& values) const
    {
        std::vector<std::string> out{};
        if (values.has_value()) {
            for (const auto& value: *values) {
                out.push_back(value.get_value());
            }
        }
        return out;
    }

    /** @brief Maps the typed filter DTO (`api_models/payouts.rs:1325-1368`) to row filters. */
    [[nodiscard]] ListFilters list_filters_from_dto(
        const app::model::payouts::PayoutListFilterConstraints& input,
        const PayoutListQuery& query
    ) const
    {
        ListFilters filters{};
        filters.payout_id = input.payout_id ? input.payout_id->get_value() : query.payout_id;
        filters.merchant_order_reference_id = input.merchant_order_reference_id.value_or("");
        filters.customer_id =
            input.customer_id ? input.customer_id->get_value() : query.customer_id;
        filters.connector = wrapper_values(input.connector);
        filters.currency = wrapper_values(input.currency);
        filters.status = status_values(input.status);
        filters.payout_method = wrapper_values(input.payout_method);
        filters.entity_type = input.entity_type ? input.entity_type->get_value() : std::string{};
        filters.offset = to_count(input.offset.get_value(), query.offset);
        filters.limit = to_count(input.limit.get_value(), query.limit);
        return filters;
    }

    /** @brief GET `/payouts/list` uses `PayoutListConstraints`, not the filter DTO. */
    [[nodiscard]] ListFilters list_filters_from_constraints(
        const app::model::payouts::PayoutListConstraints& constraints
    ) const
    {
        ListFilters filters{};
        if (constraints.customer_id.has_value()) {
            filters.customer_id = constraints.customer_id->get_value();
        }
        filters.limit = to_count(constraints.limit.get_value(), std::nullopt);
        return filters;
    }

private:
    app::context::AppContext& m_context;
};

/**
 * @brief Strict-parity notes (comment-only; see docs/hyperswitch-parity/payouts.md).
 * Line references are to the original monolith `include/app/handlers/payouts.cppm`.
 *
 * - Upstream UCS / direct / shadow connector decision tiers
 *   (`core/payouts.rs:167-418,4676-4810`) are external infrastructure. The in-process
 *   `ConnectorRouter` plus request-connector eligibility in `make_connector_decision`
 *   (`:1798-1841`) and the shadow-killed signal in `invoke_connector` (`:1868-1893`,
 *   surfaced at `:162-164`) preserve the observable decision contract. The v2
 *   `decide_unified_connector_service_payout` stub (`core/payouts.rs:4814-4826`) is
 *   `todo!()` and has no behavior to port.
 * - Temp-locker / token vault (`core/payouts.rs:730-749`) is the local key store in
 *   `store_payout_method_data` (`:1787-1796`); the external-vault PSP proxy
 *   (`core/payouts.rs:676-691`) keeps the upstream pre-confirmation status at `:134-144`.
 * - Webhook recipient resolution / `create_event_and_trigger_outgoing_webhook`
 *   (`utils.rs:1470-1537`) is the local EventBus + merchant-scoped WebhookDeliveryService in
 *   `emit_payout_webhook` (`:2251-2284`); its status-to-event mapping is an exact port of
 *   `common_enums/src/transformers.rs:2194-2215`, including statuses that emit nothing.
 * - OLAP SQL (`db/kafka_store.rs:2783,2907,2981`) is equivalent in-memory filtering/zero-fill
 *   in `available_filters` (`:2416-2450`), `filters_v2_from_mcas` (`:2509-2565`),
 *   `aggregate_response` (`:2397-2411`) and `list_response` (`:2572-2635`).
 * - The dedicated payout_link DB entity (`core/payouts.rs:808-823`) is mirrored in record
 *   metadata at `:197-201` and reflected through the typed response at `:2364-2372`.
 * - JWT `allow_platform` / `allow_connected` flags and `LockAction::NotApplicable`
 *   (`routes/payouts.rs:75-78,120-131,356-367,720-724`) are router-layer auth/lock
 *   configuration that emit no wire bytes, and the local auth context has no platform
 *   principal. `auth_mode_for` / `check_upstream_permission` / `authorize_operation`
 *   (in `input.cppm`) implement the complete observable credential/permission contract.
 *   No behavior to port.
 */
} // namespace app::handlers::payouts
