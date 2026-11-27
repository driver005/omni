export module app_payment:payment_link_runtime_common;

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

import :payment_link_input;
import :payment_link_result;

export namespace app::handlers::payment_link {

// ─────────────────────────────────────────────────────────────────────────────
// Upstream defaults (router/src/consts.rs:155-190,
// common_utils/src/consts.rs:85,164; payment_link/src/consts.rs:4-8).
// ─────────────────────────────────────────────────────────────────────────────
inline constexpr std::int64_t default_session_expiry = 15 * 60;
inline constexpr std::string_view default_background_color = "#212E46";
inline constexpr std::string_view default_merchant_logo =
    "https://live.hyperswitch.io/payment-link-assets/Merchant_placeholder.png";
inline constexpr std::string_view default_sdk_layout = "tabs";
inline constexpr std::string_view default_product_img =
    "https://live.hyperswitch.io/payment-link-assets/cart_placeholder.png";
inline constexpr std::string_view default_sdk_url =
    "https://checkout.hyperswitch.io/v0/HyperLoader.js";
inline constexpr std::string_view redirection_log_path = "logs/redirection";
inline constexpr std::string_view default_unified_code = "UE_9000";
inline constexpr std::string_view default_unified_message = "Something went wrong";

/** @brief Full PaymentLinkConfig with upstream field defaults applied. */
struct LinkConfig
{
    std::string theme{default_background_color};
    std::string return_url{};
    std::string merchant_name{};
    std::string logo{default_merchant_logo};
    std::string publishable_key{};
    std::string client_secret{};
    std::string locale{"en"};
    std::string domain{"pay.hyperswitch.io"};
    std::string sdk_layout{default_sdk_layout};
    std::string sdk_url{default_sdk_url};
    std::vector<std::string> allowed_domains{};
    bool display_sdk_only{false};
    bool enabled_saved_payment_method{false};
    bool hide_card_nickname_field{false};
    bool show_card_form_by_default{true};
    bool enable_button_only_on_form_ready{false};
    bool show_merchant_name{true};
    std::optional<std::string> background_colour{};
    std::optional<std::string> payment_button_text{};
    std::optional<std::string> custom_message_for_card_terms{};
    std::optional<std::string> custom_message_for_payment_method_types{};
    std::optional<std::string> payment_button_colour{};
    std::optional<std::string> payment_button_text_colour{};
    std::optional<std::string> payment_form_header_text{};
    std::optional<std::string> payment_methods_separator_text{};
    std::optional<bool> skip_status_screen{};
    std::optional<bool> branding_visibility{};
    std::optional<std::uint32_t> redirect_delay_seconds{};
    bool require_client_secret{false};
    bool require_allowed_domains{false};
};

struct TimeWindow
{
    std::string start{};
    std::string end{};
};

/** @brief Output of the form_payment_link_data() port (core:96-360). */
struct FormLinkData
{
    bool is_status{false};
    ::app::model::payments::PaymentLinkDetails details{};
    ::app::model::payments::PaymentLinkStatusDetails status{};
    // Untagged PaymentLinkStatusWrap serializes its inner value: either the
    // IntentStatus string or "expired" (core:249-267, api_models:14014-14020).
    std::string status_inner{};
    LinkConfig config{};
};

/**
 * @brief Shared business helpers for the payment_link sub-runtimes.
 *
 * Holds the AppContext reference and the upstream ports (config resolution,
 * persistence lookups, XSS/format validation, wire rendering). No HTTP types:
 * handlers pass plain Input structs and receive plain Result structs.
 */
class PaymentLinkRuntimeSupport
{
public:
    explicit PaymentLinkRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] std::int64_t now_epoch() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    }

    [[nodiscard]] std::string iso_utc(std::int64_t epoch_seconds) const
    {
        constexpr std::int64_t seconds_per_day{86'400};
        std::int64_t days = epoch_seconds / seconds_per_day;
        std::int64_t seconds_of_day = epoch_seconds % seconds_per_day;
        if (seconds_of_day < 0) {
            seconds_of_day += seconds_per_day;
            --days;
        }
        days += 719'468;
        const std::int64_t era = (days >= 0 ? days : days - 146'096) / 146'097;
        const std::int64_t doe = days - era * 146'097;
        const std::int64_t yoe = (doe - doe / 1'460 + doe / 36'524 - doe / 146'096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format(
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
            year,
            month,
            day,
            seconds_of_day / 3'600,
            (seconds_of_day % 3'600) / 60,
            seconds_of_day % 60
        );
    }

    [[nodiscard]] std::optional<std::int64_t> iso_to_epoch(std::string_view value) const
    {
        if (value.size() < 10) {
            return std::nullopt;
        }
        int year{};
        int month{};
        int day{};
        if (std::from_chars(value.data(), value.data() + 4, year).ec != std::errc{} ||
            value[4] != '-') {
            return std::nullopt;
        }
        if (std::from_chars(value.data() + 5, value.data() + 7, month).ec != std::errc{} ||
            value[7] != '-') {
            return std::nullopt;
        }
        if (std::from_chars(value.data() + 8, value.data() + 10, day).ec != std::errc{}) {
            return std::nullopt;
        }
        year -= (month <= 2) ? 1 : 0;
        const std::int64_t era = (year >= 0 ? year : year - 399) / 400;
        const std::int64_t yoe = year - era * 400;
        const std::int64_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
        const std::int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        const std::int64_t days = era * 146'097 + doe - 719'468;
        return days * 86'400;
    }

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        std::size_t begin{0};
        std::size_t end{value.size()};
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) {
            ++begin;
        }
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
            --end;
        }
        return std::string{value.substr(begin, end - begin)};
    }

    [[nodiscard]] std::size_t parse_size(std::string_view value) const
    {
        std::size_t parsed{};
        const auto converted = std::from_chars(value.data(), value.data() + value.size(), parsed);
        return converted.ec == std::errc{} && converted.ptr == value.data() + value.size() ? parsed
                                                                                           : 0U;
    }

    [[nodiscard]] bool parse_bool(std::string_view value, bool fallback) const
    {
        std::string lower{value};
        for (char& c: lower) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (lower == "true" || lower == "1") {
            return true;
        }
        if (lower == "false" || lower == "0") {
            return false;
        }
        return fallback;
    }

    /** @brief Upstream list defaults to 30 days, rejects end<=start, caps at 90 days. */
    [[nodiscard]] core::behavior::Outcome<TimeWindow>
    time_window(std::optional<std::string> raw) const
    {
        const auto now = now_epoch();
        std::int64_t start = now - 30 * 86'400;
        std::int64_t end = now;
        if (raw) {
            std::string value = trim(*raw);
            if (value.size() >= 2 && value.front() == '[' && value.back() == ']') {
                value = value.substr(1, value.size() - 2);
            }
            const std::size_t comma = value.find(',');
            if (comma == std::string::npos) {
                return std::unexpected{
                    make_error(core::error::AppErrorKind::INVALID_REQUEST, "invalid time range")
                };
            }
            const auto parsed_start = iso_to_epoch(trim(value.substr(0, comma)));
            const auto parsed_end = iso_to_epoch(trim(value.substr(comma + 1)));
            if (!parsed_start || !parsed_end) {
                return std::unexpected{
                    make_error(core::error::AppErrorKind::INVALID_REQUEST, "invalid time range")
                };
            }
            start = *parsed_start;
            end = *parsed_end;
        }
        if (start >= end) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "time range end must be after start"
            )};
        }
        if (end - start > 90 * 86'400) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::INVALID_REQUEST, "time range exceeds 90 days")
            };
        }
        return TimeWindow{iso_utc(start), iso_utc(end)};
    }

    [[nodiscard]] bool
    within_range(std::string_view created, const TimeWindow& window) const noexcept
    {
        if (created.empty()) {
            return true;
        }
        return created >= window.start && created <= window.end;
    }

    [[nodiscard]] std::string metadata_value(std::string_view metadata, std::string_view key) const
    {
        const std::string marker{std::string{key} + "="};
        const std::size_t pos = metadata.find(marker);
        if (pos == std::string_view::npos) {
            return {};
        }
        const std::size_t begin = pos + marker.size();
        std::size_t end = metadata.find(';', begin);
        if (end == std::string_view::npos) {
            end = metadata.find(',', begin);
        }
        if (end == std::string_view::npos) {
            end = metadata.size();
        }
        return std::string{metadata.substr(begin, end - begin)};
    }

    [[nodiscard]] std::string config_value(
        std::string_view config_namespace,
        std::string_view key,
        std::string_view context,
        std::string_view fallback
    ) const
    {
        if (const auto record = m_context.superposition().resolve(config_namespace, key, context)) {
            if (!record->get_value().empty()) {
                return record->get_value();
            }
        }
        if (const auto value = m_context.config_store().get_value(config_namespace, key)) {
            if (!value->empty()) {
                return *value;
            }
        }
        return std::string{fallback};
    }

    /**
     * @brief get_payment_link_config_based_on_priority() + extract_payment_link_config()
     * substitute (core:96-169,699-859). Priority is payment-create metadata >
     * profile scope > merchant scope > default config > built-in upstream default.
     */
    [[nodiscard]] LinkConfig resolve_config(
        std::string_view merchant_id,
        std::string_view profile_id,
        std::string_view payment_id,
        std::string_view metadata
    ) const
    {
        const std::string merchant_context =
            merchant_id.empty() ? "default" : std::string{merchant_id};
        const std::string scoped_context =
            profile_id.empty() ? merchant_context
                               : std::string{merchant_id} + "/" + std::string{profile_id};
        const auto pick = [&](std::string_view key, std::string_view fallback)
        {
            std::string value = metadata_value(metadata, key);
            if (value.empty()) {
                value = config_value("payment_link", key, scoped_context, "");
            }
            if (value.empty()) {
                value = config_value("payment_link", key, merchant_context, "");
            }
            if (value.empty()) {
                value = config_value("payment_link", key, "default", "");
            }
            return value.empty() ? std::string{fallback} : value;
        };

        LinkConfig config{};
        config.theme = pick("theme", std::string{default_background_color});
        config.logo = pick("logo", std::string{default_merchant_logo});
        config.locale = pick("locale", "en");
        config.sdk_layout = pick("sdk_layout", std::string{default_sdk_layout});
        config.domain = pick("domain_name", "pay.hyperswitch.io");
        config.sdk_url =
            config_value("payment_link", "sdk_url", "default", std::string{default_sdk_url});
        config.return_url = pick("return_url", "");
        config.display_sdk_only = parse_bool(pick("display_sdk_only", "false"), false);
        config.enabled_saved_payment_method =
            parse_bool(pick("enabled_saved_payment_method", "false"), false);
        config.hide_card_nickname_field =
            parse_bool(pick("hide_card_nickname_field", "false"), false);
        config.show_card_form_by_default =
            parse_bool(pick("show_card_form_by_default", "true"), true);
        config.enable_button_only_on_form_ready =
            parse_bool(pick("enable_button_only_on_form_ready", "false"), false);
        config.show_merchant_name = parse_bool(pick("show_merchant_name", "true"), true);
        config.require_client_secret = parse_bool(pick("require_client_secret", "false"), false);
        config.require_allowed_domains =
            parse_bool(pick("require_allowed_domains", "false"), false);

        const auto optional_string = [&](std::string_view key) -> std::optional<std::string>
        {
            const std::string value = pick(key, "");
            return value.empty() ? std::nullopt : std::optional<std::string>{value};
        };
        config.background_colour = optional_string("background_colour");
        config.payment_button_text = optional_string("payment_button_text");
        config.custom_message_for_card_terms = optional_string("custom_message_for_card_terms");
        config.custom_message_for_payment_method_types =
            optional_string("custom_message_for_payment_method_types");
        config.payment_button_colour = optional_string("payment_button_colour");
        config.payment_button_text_colour = optional_string("payment_button_text_colour");
        config.payment_form_header_text = optional_string("payment_form_header_text");
        config.payment_methods_separator_text = optional_string("payment_methods_separator_text");
        const std::string skip = pick("skip_status_screen", "");
        if (!skip.empty()) {
            config.skip_status_screen = parse_bool(skip, false);
        }
        const std::string branding = pick("branding_visibility", "");
        if (!branding.empty()) {
            config.branding_visibility = parse_bool(branding, true);
        }
        const std::string delay = pick("redirect_delay_seconds", "");
        if (!delay.empty()) {
            config.redirect_delay_seconds = static_cast<std::uint32_t>(parse_size(delay));
        }

        const std::string domains = pick("allowed_domains", "");
        std::size_t begin{0};
        while (begin <= domains.size()) {
            const std::size_t comma = domains.find(',', begin);
            const std::string part = trim(
                std::string_view{domains}.substr(
                    begin,
                    comma == std::string::npos ? std::string_view::npos : comma - begin
                )
            );
            if (!part.empty()) {
                config.allowed_domains.push_back(part);
            }
            if (comma == std::string::npos) {
                break;
            }
            begin = comma + 1;
        }

        const auto account = m_context.repository().find_merchant_account_by_string(merchant_id);
        if (account) {
            if (!account->get_merchant_name().empty()) {
                config.merchant_name = account->get_merchant_name();
            }
            if (!account->get_return_url().empty() && config.return_url.empty()) {
                config.return_url = account->get_return_url();
            }
            if (!account->get_publishable_key().empty()) {
                config.publishable_key = account->get_publishable_key();
            }
        }
        if (config.publishable_key.empty()) {
            if (auto connector = m_context.connector_router().select(merchant_id)) {
                config.publishable_key = "pk_" + std::string{connector->name()};
            }
        }
        if (config.publishable_key.empty()) {
            config.publishable_key = "pk_test_" + std::string{merchant_id};
        }

        std::string secret_key{"hyperswitch-payment-link-client-secret"};
        if (const auto resolved =
                m_context.secret_resolver().resolve("payment_link_client_secret", "default")) {
            if (!resolved->empty()) {
                secret_key = *resolved;
            }
        }
        config.client_secret =
            "cs_" + ::app::crypto::KeyedHasher{std::move(secret_key)}.hash_with_domain(
                        ::app::crypto::HashDomain::ClientSecret,
                        std::string{merchant_id} + ":" + std::string{payment_id}
                    );
        return config;
    }

    /** @brief ValidateXSSOrSQLi substitute over the resolved config (core:850-856). */
    [[nodiscard]] bool unsafe_text(std::string_view value) const
    {
        std::string lower{value};
        for (char& c: lower) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        for (const std::string_view needle:
             {"<script",
              "javascript:",
              "onerror=",
              "onload=",
              "<iframe",
              "<img",
              "' or ",
              "\" or ",
              "drop table",
              "; drop",
              "--",
              "</style>",
              "expression("}) {
            if (lower.find(needle) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool unsafe_config(const LinkConfig& config) const
    {
        for (const std::string_view value:
             {std::string_view{config.theme},
              std::string_view{config.logo},
              std::string_view{config.return_url},
              std::string_view{config.sdk_layout},
              std::string_view{config.publishable_key}}) {
            if (unsafe_text(value)) {
                return true;
            }
        }
        for (const auto& domain: config.allowed_domains) {
            if (unsafe_text(domain)) {
                return true;
            }
        }
        for (const auto& optional:
             {config.background_colour,
              config.payment_button_text,
              config.custom_message_for_card_terms,
              config.custom_message_for_payment_method_types,
              config.payment_button_colour,
              config.payment_button_text_colour,
              config.payment_form_header_text,
              config.payment_methods_separator_text}) {
            if (optional && unsafe_text(*optional)) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::string capitalized(std::string value) const
    {
        if (!value.empty()) {
            value.front() =
                static_cast<char>(std::toupper(static_cast<unsigned char>(value.front())));
        }
        return value;
    }

    /** @brief check_payment_link_invalid_conditions() (core:221-233,874-879). */
    [[nodiscard]] bool terminal_status(std::string_view status) const
    {
        static constexpr std::string_view blocked[] = {
            "cancelled",
            "canceled",
            "failed",
            "processing",
            "requires_capture",
            "requires_merchant_action",
            "succeeded",
            "partially_captured",
            "requires_customer_action",
            "paid"
        };
        for (const auto value: blocked) {
            if (status == value) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool is_expired(std::string_view raw) const
    {
        if (raw.empty()) {
            return false;
        }
        std::int64_t expiry_seconds{};
        const auto converted = std::from_chars(raw.data(), raw.data() + raw.size(), expiry_seconds);
        if (converted.ec != std::errc{} || converted.ptr != raw.data() + raw.size()) {
            return false;
        }
        return expiry_seconds < now_epoch();
    }

    /** @brief StringMajorUnitForCore substitute with zero-decimal currency handling. */
    [[nodiscard]] std::string
    major_unit_amount(std::int64_t amount, std::string_view currency) const
    {
        constexpr std::string_view zero_decimal[] = {"JPY", "KRW", "VND", "CLP", "ISK", "UGX"};
        bool zero_decimals = false;
        for (const auto code: zero_decimal) {
            if (code == currency) {
                zero_decimals = true;
                break;
            }
        }
        const bool negative = amount < 0;
        const std::int64_t magnitude = negative ? -amount : amount;
        const std::string_view sign = negative ? "-" : "";
        if (zero_decimals) {
            return std::format("{}{}", sign, magnitude);
        }
        return std::format("{}{}.{:02d}", sign, magnitude / 100, magnitude % 100);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Upstream rendering helpers (payment_link/src/js_generator.rs,
    // css_generator.rs, meta_tags.rs; core/payment_link.rs:43-52,524-541).
    // ─────────────────────────────────────────────────────────────────────────

    /** @brief urlencoding::encode: percent-encode every byte outside RFC 3986 unreserved. */
    [[nodiscard]] std::string url_encode(std::string_view value) const
    {
        static constexpr char hex[] = "0123456789ABCDEF";
        std::string out{};
        out.reserve(value.size());
        for (const unsigned char c: value) {
            const bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                                    (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
                                    c == '~';
            if (unreserved) {
                out.push_back(static_cast<char>(c));
            } else {
                out.push_back('%');
                out.push_back(hex[c >> 4]);
                out.push_back(hex[c & 0x0F]);
            }
        }
        return out;
    }

    [[nodiscard]] std::string html_escape(std::string_view value) const
    {
        std::string out{};
        for (const char c: value) {
            switch (c) {
                case '"':
                    out += "&quot;";
                    break;
                case '\'':
                    out += "&#x27;";
                    break;
                case '&':
                    out += "&amp;";
                    break;
                case '<':
                    out += "&lt;";
                    break;
                case '>':
                    out += "&gt;";
                    break;
                default:
                    out.push_back(c);
                    break;
            }
        }
        return out;
    }

    /** @brief payment_link/src/js_generator.rs:86-101. */
    template<typename T>
    [[nodiscard]] std::string get_js_script(const T& details) const
    {
        const std::string json = serde::Ser::encode_json(details);
        // convert_custom_message_keys_to_camel() is a no-op for the local DTO shape.
        return std::format("window.__PAYMENT_DETAILS = '{}';", url_encode(json));
    }

    /**
     * @brief Upstream PaymentLinkStatusWrap is #[serde(untagged)]
     * (api_models/payments.rs:14014-14020): the wire carries the inner value,
     * not the local enum name. Override the typed document's `status` field.
     */
    [[nodiscard]] serde::Value status_details_document(
        const ::app::model::payments::PaymentLinkStatusDetails& details,
        std::string_view inner
    ) const
    {
        serde::Value document = PaymentLinkDocument::from(details);
        if (auto* object = std::get_if<serde::Value::Object>(&document.get())) {
            for (auto& field: *object) {
                if (field.first == "status") {
                    field.second = serde::Value{std::string{inner}};
                    break;
                }
            }
        }
        return document;
    }

    [[nodiscard]] std::string get_status_js_script(
        const ::app::model::payments::PaymentLinkStatusDetails& details,
        std::string_view inner
    ) const
    {
        const std::string json = serde::Ser::encode_json(status_details_document(details, inner));
        return std::format("window.__PAYMENT_DETAILS = '{}';", url_encode(json));
    }

    /** @brief payment_link/src/css_generator.rs:25-91. */
    [[nodiscard]] std::string sanitize_color(std::string_view raw) const
    {
        const std::string value = trim(raw);
        if (value.empty()) {
            return {};
        }
        if (value.starts_with('#')) {
            const std::string hex = value.substr(1);
            if ((hex.size() == 3 || hex.size() == 4 || hex.size() == 6 || hex.size() == 8) &&
                std::all_of(
                    hex.begin(),
                    hex.end(),
                    [](unsigned char c)
                    {
                        return std::isxdigit(c) != 0;
                    }
                )) {
                return value;
            }
            return {};
        }
        if ((value.starts_with("rgb(") || value.starts_with("rgba(") || value.starts_with("hsl(") ||
             value.starts_with("hsla(")) &&
            value.ends_with(')')) {
            const auto start = value.find('(');
            const std::string inner = value.substr(start + 1, value.size() - start - 2);
            const bool valid = std::all_of(
                inner.begin(),
                inner.end(),
                [](unsigned char c)
                {
                    return std::isdigit(c) || c == ',' || c == '.' || c == '%' || c == ' ';
                }
            );
            return valid ? value : std::string{};
        }
        const bool named = std::all_of(
            value.begin(),
            value.end(),
            [](unsigned char c)
            {
                return std::isalpha(c) || c == '-';
            }
        );
        return named ? value : std::string{};
    }

    [[nodiscard]] std::string get_css_script(const LinkConfig& config) const
    {
        const std::string raw = config.background_colour.value_or(config.theme);
        const std::string color = sanitize_color(raw);
        const std::string primary = color.empty() ? std::string{"#4E6ADD"} : color;
        std::string css = std::format(":root {{\n      --primary-color: {};\n    }}", primary);
        // payment_link_ui_rules has no local store column; when one is supplied
        // via config it is emitted verbatim after the color-scheme block.
        const std::string ui_rules =
            config_value("payment_link", "payment_link_ui_rules", "default", "");
        if (!ui_rules.empty()) {
            css += "\n" + ui_rules;
        }
        return css;
    }

    [[nodiscard]] std::string
    get_meta_tags_html(const ::app::model::payments::PaymentLinkDetails& details) const
    {
        const std::string description = details.get_merchant_description()
                                            ? *details.get_merchant_description()
                                            : std::string{};
        return std::format(
            "<meta property=\"og:title\" content=\"Payment request from {}\"/>\n"
            "        <meta property=\"og:description\" content=\"{}\"/>",
            html_escape(details.get_merchant_name()),
            html_escape(description)
        );
    }

    [[nodiscard]] std::string get_redirection_log_endpoint() const
    {
        std::string base = config_value("payment_link", "base_url", "default", "");
        if (base.empty()) {
            base = m_context.config_store()
                       .get_value("application", "base_url")
                       .value_or(std::string{"https://api.hyperswitch.io"});
        }
        while (base.ends_with('/')) {
            base.pop_back();
        }
        return base + "/" + std::string{redirection_log_path};
    }

    // ─────────────────────────────────────────────────────────────────────────
    // form_payment_link_data() port (core:96-360) and status route port
    // (core:892-1073).
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::optional<::app::store::payment_attempts::PaymentAttemptRecord>
    find_attempt(std::string_view payment_id, std::string_view merchant_id) const
    {
        for (const auto& attempt: m_context.repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == payment_id &&
                (merchant_id.empty() || attempt.get_merchant_id() == merchant_id)) {
                return attempt;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief validate_order_details() (core:633-687). The source order details
     * live in intent metadata (`order_details=`); each entry is
     * `name|quantity|amount_minor|img`. Amounts are converted to major units,
     * product names capitalized, and a default image applied when absent.
     */
    [[nodiscard]] std::optional<std::vector<::app::model::payments::OrderDetailsWithStringAmount>>
    validate_order_details(std::string_view metadata, std::string_view currency) const
    {
        const std::string raw = metadata_value(metadata, "order_details");
        if (raw.empty()) {
            return std::nullopt;
        }
        std::vector<::app::model::payments::OrderDetailsWithStringAmount> out{};
        std::size_t begin{0};
        while (begin <= raw.size()) {
            std::size_t end = raw.find(';', begin);
            if (end == std::string::npos) {
                end = raw.size();
            }
            const std::string entry = std::string{raw.substr(begin, end - begin)};
            std::vector<std::string> parts{};
            std::size_t field_begin{0};
            while (field_begin <= entry.size()) {
                const std::size_t bar = entry.find('|', field_begin);
                parts.push_back(entry.substr(
                    field_begin,
                    bar == std::string::npos ? std::string::npos : bar - field_begin
                ));
                if (bar == std::string::npos) {
                    break;
                }
                field_begin = bar + 1;
            }
            if (parts.size() >= 3) {
                ::app::model::payments::OrderDetailsWithStringAmount order{};
                order.set_product_name(capitalized(parts[0]));
                order.set_quantity(
                    static_cast<std::uint16_t>(
                        std::clamp(parse_size(parts[1]), std::size_t{0}, std::size_t{65'535})
                    )
                );
                ::app::model::payments::StringMajorUnitValue amount{};
                amount.set_value(
                    major_unit_amount(static_cast<std::int64_t>(parse_size(parts[2])), currency)
                );
                order.set_amount(std::move(amount));
                order.set_product_img_link(
                    parts.size() >= 4 && !parts[3].empty()
                        ? std::optional<std::string>{parts[3]}
                        : std::optional<std::string>{std::string{default_product_img}}
                );
                out.push_back(std::move(order));
            }
            if (end == raw.size()) {
                break;
            }
            begin = end + 1;
        }
        return out.empty() ? std::nullopt : std::optional{std::move(out)};
    }

    [[nodiscard]] core::behavior::Outcome<FormLinkData>
    form_payment_link_data(std::string_view merchant_id, std::string_view payment_id) const
    {
        // find_payment_intent_by_payment_id_processor_merchant_id (core:105-113).
        auto intent = find_intent(payment_id);
        if (!intent || (!merchant_id.empty() && !intent->get_merchant_id().empty() &&
                        intent->get_merchant_id() != merchant_id)) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found")
            };
        }
        // get_required_value("payment_link_id") (core:115-118).
        const auto link = find_link_by_payment(merchant_id, payment_id);
        const std::string payment_link_id = link ? link->get_link_id() : std::string{payment_id};
        if (link && !link->get_merchant_id().empty() && link->get_merchant_id() != merchant_id) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::NOT_FOUND, "payment link not found")
            };
        }

        const auto config = resolve_config(
            merchant_id,
            link ? link->get_profile_id() : std::string{},
            payment_link_id,
            intent->get_metadata()
        );
        if (unsafe_config(config)) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "unsafe payment link configuration"
            )};
        }

        // validate_sdk_requirements() (core:543-555): currency + client secret.
        const std::string currency =
            link && !link->get_currency().empty() ? link->get_currency() : intent->get_currency();
        if (currency.empty()) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::INVALID_REQUEST, "currency")
            };
        }
        if (config.client_secret.empty()) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::INVALID_REQUEST, "client_secret")
            };
        }

        const std::int64_t amount_minor = link ? link->get_amount() : intent->get_amount();
        const std::string amount = major_unit_amount(amount_minor, currency);
        auto order_details = validate_order_details(intent->get_metadata(), currency);

        // session_expiry = fulfilment_time.unwrap_or(created_at + DEFAULT_SESSION_EXPIRY)
        // (core:211-215).
        std::string session_expiry = link ? link->get_expires_at() : std::string{};
        if (session_expiry.empty()) {
            const auto created_epoch = iso_to_epoch(intent->get_created_at()).value_or(now_epoch());
            session_expiry = iso_utc(created_epoch + default_session_expiry);
        }

        const std::string merchant_name = capitalized(config.merchant_name);
        const bool terminal_intent = terminal_status(intent->get_status());
        const bool expired = is_expired(link ? link->get_expires_at() : std::string{});

        FormLinkData result{};
        result.config = config;

        if (terminal_intent || expired) {
            // core:246-307: status page with redirect=false.
            result.is_status = true;
            ::app::model::payments::PaymentLinkStatusDetails& details = result.status;
            ::app::model::payments::StringMajorUnitValue amount_value{};
            amount_value.set_value(amount);
            details.set_amount(std::move(amount_value));
            ::app::model::payments::CurrencyValue currency_value{};
            currency_value.set_value(currency);
            details.set_currency(std::move(currency_value));
            ::app::model::payments::PaymentIdValue id{};
            id.set_value(std::string{payment_id});
            details.set_payment_id(std::move(id));
            details.set_merchant_name(merchant_name);
            details.set_merchant_logo(config.logo);
            ::app::model::payments::PrimitiveDateTimeValue created{};
            created.set_value(link ? link->get_created_at() : intent->get_created_at());
            details.set_created(std::move(created));
            // core:249-267: terminal intent wins; otherwise an expired link.
            details.set_status(
                terminal_intent ? ::app::model::payments::PaymentLinkStatusWrap::IntentStatus
                                : ::app::model::payments::PaymentLinkStatusWrap::PaymentLinkStatus
            );
            result.status_inner = terminal_intent ? intent->get_status() : std::string{"expired"};
            details.set_redirect(false);
            details.set_theme(config.theme);
            details.set_return_url(config.return_url);
            details.set_locale(config.locale);
            const auto attempt = find_attempt(payment_id, merchant_id);
            if (attempt) {
                if (!attempt->get_error_code().empty()) {
                    details.set_error_code(attempt->get_error_code());
                }
                if (!attempt->get_error_message().empty()) {
                    details.set_error_message(attempt->get_error_message());
                }
                if (!attempt->get_error_code().empty()) {
                    details.set_unified_code(attempt->get_error_code());
                }
                if (!attempt->get_error_message().empty() &&
                    !unsafe_text(attempt->get_error_message())) {
                    details.set_unified_message(attempt->get_error_message());
                }
            }
            if (expired && !terminal_intent) {
                details.set_error_code(std::string{"payment_link_expired"});
                details.set_error_message(std::string{"This payment link has expired."});
            }
            details.set_redirect_delay_seconds(config.redirect_delay_seconds);
            return result;
        }

        // core:309-353: PaymentLinkDetails.
        ::app::model::payments::PaymentLinkDetails& details = result.details;
        ::app::model::payments::StringMajorUnitValue amount_value{};
        amount_value.set_value(amount);
        details.set_amount(std::move(amount_value));
        ::app::model::payments::CurrencyValue currency_value{};
        currency_value.set_value(currency);
        details.set_currency(std::move(currency_value));
        details.set_pub_key(config.publishable_key);
        details.set_client_secret(config.client_secret);
        ::app::model::payments::PaymentIdValue id{};
        id.set_value(std::string{payment_id});
        details.set_payment_id(std::move(id));
        ::app::model::payments::PrimitiveDateTimeValue expiry{};
        expiry.set_value(session_expiry);
        details.set_session_expiry(std::move(expiry));
        details.set_merchant_logo(config.logo);
        details.set_return_url(config.return_url);
        details.set_merchant_name(merchant_name);
        details.set_order_details(std::move(order_details));
        details.set_max_items_visible_after_collapse(3);
        details.set_theme(config.theme);
        if (!intent->get_description().empty() && !unsafe_text(intent->get_description())) {
            details.set_merchant_description(intent->get_description());
        }
        details.set_sdk_layout(config.sdk_layout);
        details.set_display_sdk_only(config.display_sdk_only);
        details.set_hide_card_nickname_field(config.hide_card_nickname_field);
        details.set_show_card_form_by_default(config.show_card_form_by_default);
        details.set_locale(config.locale);
        details.set_branding_visibility(config.branding_visibility);
        details.set_payment_button_text(config.payment_button_text);
        details.set_skip_status_screen(config.skip_status_screen);
        details.set_custom_message_for_card_terms(config.custom_message_for_card_terms);
        details.set_payment_button_colour(config.payment_button_colour);
        details.set_background_colour(config.background_colour);
        details.set_payment_button_text_colour(config.payment_button_text_colour);
        ::app::model::payments::IntentStatusValue status{};
        status.set_value(intent->get_status());
        details.set_status(std::move(status));
        details.set_enable_button_only_on_form_ready(config.enable_button_only_on_form_ready);
        details.set_payment_form_header_text(config.payment_form_header_text);
        details.set_show_merchant_name(config.show_merchant_name);
        details.set_payment_methods_separator_text(config.payment_methods_separator_text);
        return result;
    }

    /** @brief get_payment_link_status() (core:892-1073) for the status route. */
    [[nodiscard]] core::behavior::Outcome<std::tuple<::app::model::payments::PaymentLinkStatusDetails, std::string, std::string>>
    build_status_page_data(std::string_view merchant_id, std::string_view payment_id) const
    {
        auto intent = find_intent(payment_id);
        if (!intent || (!merchant_id.empty() && !intent->get_merchant_id().empty() &&
                        intent->get_merchant_id() != merchant_id)) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found")
            };
        }
        const auto attempt = find_attempt(payment_id, merchant_id);
        if (!attempt) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::NOT_FOUND, "payment not found")
            };
        }
        const auto link = find_link_by_payment(merchant_id, payment_id);
        const std::string payment_link_id = link ? link->get_link_id() : std::string{payment_id};

        const auto config = resolve_config(
            merchant_id,
            link ? link->get_profile_id() : std::string{},
            payment_link_id,
            intent->get_metadata()
        );
        if (unsafe_config(config)) {
            return std::unexpected{make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "unsafe payment link configuration"
            )};
        }

        // currency is required (core:978-983).
        const std::string currency =
            link && !link->get_currency().empty() ? link->get_currency() : intent->get_currency();
        if (currency.empty()) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::INVALID_REQUEST, "currency")
            };
        }
        // Amount is taken from the active attempt (core:985-991).
        const std::int64_t amount_minor =
            attempt->get_amount() != 0 ? attempt->get_amount() : intent->get_amount();

        const std::string unified_code = attempt->get_error_code().empty()
                                             ? std::string{default_unified_code}
                                             : attempt->get_error_code();
        const std::string unified_message = attempt->get_error_message().empty()
                                                ? std::string{default_unified_message}
                                                : attempt->get_error_message();

        ::app::model::payments::PaymentLinkStatusDetails details{};
        ::app::model::payments::StringMajorUnitValue amount{};
        amount.set_value(major_unit_amount(amount_minor, currency));
        details.set_amount(std::move(amount));
        ::app::model::payments::CurrencyValue currency_value{};
        currency_value.set_value(currency);
        details.set_currency(std::move(currency_value));
        ::app::model::payments::PaymentIdValue id{};
        id.set_value(std::string{payment_id});
        details.set_payment_id(std::move(id));
        details.set_merchant_name(capitalized(config.merchant_name));
        details.set_merchant_logo(config.logo);
        ::app::model::payments::PrimitiveDateTimeValue created{};
        created.set_value(link ? link->get_created_at() : intent->get_created_at());
        details.set_created(std::move(created));
        // core:1041-1061: the status route always renders IntentStatus/redirect=true.
        details.set_status(::app::model::payments::PaymentLinkStatusWrap::IntentStatus);
        details.set_redirect(true);
        if (!attempt->get_error_code().empty()) {
            details.set_error_code(attempt->get_error_code());
        }
        if (!attempt->get_error_message().empty()) {
            details.set_error_message(attempt->get_error_message());
        }
        details.set_theme(config.theme);
        details.set_return_url(config.return_url);
        details.set_locale(config.locale);
        details.set_unified_code(unified_code);
        details.set_unified_message(unified_message);
        details.set_redirect_delay_seconds(config.redirect_delay_seconds);

        return std::tuple{std::move(details), get_css_script(config), intent->get_status()};
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Upstream ApplicationResponse wire shapes (typed documents).
    // ─────────────────────────────────────────────────────────────────────────

    /** @brief payment_link::PaymentLinkFormData (types.rs:11-17). */
    [[nodiscard]] serde::Value payment_form_data_document_fields(
        std::string_view js,
        std::string_view css,
        std::string_view sdk_url,
        std::string_view meta_tags
    ) const
    {
        serde::Value::Object document{};
        document["js_script"] = serde::Value{std::string{js}};
        document["css_script"] = serde::Value{std::string{css}};
        document["sdk_url"] = serde::Value{std::string{sdk_url}};
        document["html_meta_tags"] = serde::Value{std::string{meta_tags}};
        document["redirection_log_endpoint"] = serde::Value{get_redirection_log_endpoint()};
        return serde::Value{std::move(document)};
    }

    template<typename T>
    [[nodiscard]] serde::Value payment_form_data_document(
        const T& details,
        std::string_view css,
        std::string_view sdk_url
    ) const
    {
        return payment_form_data_document_fields(
            get_js_script(details),
            css,
            sdk_url,
            get_meta_tags_html(details)
        );
    }

    /** @brief payment_link::PaymentLinkStatusData (types.rs:20-24). */
    [[nodiscard]] serde::Value status_page_data_document(
        const ::app::model::payments::PaymentLinkStatusDetails& details,
        std::string_view css,
        std::string_view status_inner
    ) const
    {
        serde::Value::Object document{};
        document["js_script"] = serde::Value{get_status_js_script(details, status_inner)};
        document["css_script"] = serde::Value{std::string{css}};
        document["redirection_log_endpoint"] = serde::Value{get_redirection_log_endpoint()};
        return serde::Value{std::move(document)};
    }

    /**
     * @brief SecurePaymentLinkDetails wire shape (api_models; core:399-421).
     * Built as a typed document; the caller encodes it into the secure js script.
     */
    [[nodiscard]] serde::Value secure_payment_link_details_document(
        const ::app::model::payments::PaymentLinkDetails& d,
        const LinkConfig& config
    ) const
    {
        const auto opt_string = [](const std::optional<std::string>& value)
        {
            return value ? serde::Value{std::string{*value}} : serde::Value{};
        };
        const auto opt_bool = [](const std::optional<bool>& value)
        {
            return value ? serde::Value{*value} : serde::Value{};
        };
        serde::Value::Object document{};
        document["enabled_saved_payment_method"] =
            serde::Value{config.enabled_saved_payment_method};
        document["hide_card_nickname_field"] = serde::Value{config.hide_card_nickname_field};
        document["show_card_form_by_default"] = serde::Value{config.show_card_form_by_default};
        document["payment_link_details"] = PaymentLinkDocument::from(d);
        document["payment_button_text"] = opt_string(config.payment_button_text);
        document["skip_status_screen"] = opt_bool(config.skip_status_screen);
        document["custom_message_for_card_terms"] =
            opt_string(config.custom_message_for_card_terms);
        document["custom_message_for_payment_method_types"] =
            opt_string(config.custom_message_for_payment_method_types);
        document["payment_button_colour"] = opt_string(config.payment_button_colour);
        document["payment_button_text_colour"] = opt_string(config.payment_button_text_colour);
        document["background_colour"] = opt_string(config.background_colour);
        document["sdk_ui_rules"] = serde::Value{};
        document["enable_button_only_on_form_ready"] =
            serde::Value{config.enable_button_only_on_form_ready};
        document["payment_form_header_text"] = opt_string(config.payment_form_header_text);
        document["payment_form_label_type"] = serde::Value{};
        document["show_card_terms"] = serde::Value{};
        document["color_icon_card_cvc_error"] = serde::Value{};
        document["payment_methods_separator_text"] =
            opt_string(config.payment_methods_separator_text);
        return serde::Value{std::move(document)};
    }

    [[nodiscard]] serde::Value generic_links_document(
        const std::vector<std::string>& allowed_domains,
        serde::Value data,
        std::string_view locale
    ) const
    {
        serde::Value::Array domains{};
        domains.reserve(allowed_domains.size());
        for (const auto& domain: allowed_domains) {
            domains.push_back(serde::Value{domain});
        }
        serde::Value::Object document{};
        document["allowed_domains"] = serde::Value{std::move(domains)};
        document["data"] = std::move(data);
        document["locale"] = serde::Value{std::string{locale}};
        return serde::Value{std::move(document)};
    }

    // ─────────────────────────────────────────────────────────────────────────
    // validator::validate_secure_payment_link_render_request (validator:12-109).
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::string host_from_url(std::string_view url) const
    {
        const auto scheme = url.find("://");
        std::size_t start{0};
        if (scheme != std::string_view::npos) {
            start = scheme + 3;
        }
        std::size_t end = url.find('/', start);
        if (end == std::string_view::npos) {
            end = url.size();
        }
        std::string authority{url.substr(start, end - start)};
        const auto at = authority.rfind('@');
        if (at != std::string::npos) {
            authority.erase(0, at + 1);
        }
        return authority;
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_secure_render_request(
        std::string_view payment_link_id,
        const LinkConfig& config,
        const SecureLinkHeaders& headers
    ) const
    {
        // allowed_domains must be present and non-empty (validator:18-24,
        // core:434-453).
        if (config.allowed_domains.empty()) {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "secure payment link was not generated for " + std::string{payment_link_id} +
                    ": missing allowed_domains"
            );
        }
        // sec-fetch-dest must be "iframe" (validator:33-54).
        if (headers.sec_fetch_dest != "iframe") {
            return make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "access to payment_link is forbidden when sec-fetch-dest is not iframe"
            );
        }
        // origin or referer (validator:56-95).
        std::string origin = headers.origin;
        if (origin.empty()) {
            origin = headers.referer;
        }
        if (origin.empty()) {
            return make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "access to payment_link is forbidden when origin/referer is missing"
            );
        }
        const std::string domain = host_from_url(origin);
        if (domain.empty()) {
            return make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "invalid origin/referer for payment_link"
            );
        }
        for (const auto& allowed: config.allowed_domains) {
            if (domain == allowed || (allowed.starts_with('.') && domain.ends_with(allowed))) {
                return std::nullopt;
            }
        }
        return make_error(
            core::error::AppErrorKind::FORBIDDEN,
            "access to payment_link is forbidden from " + domain
        );
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Persistence / list helpers.
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] std::optional<::app::store::payment_links::PaymentLinkRecord>
    find_link_by_id(std::string_view id) const
    {
        if (auto record = m_context.repository().find_payment_link_by_string(id)) {
            return record;
        }
        for (const auto& record: m_context.repository().list_payment_links()) {
            if (record.get_link_id() == id ||
                (!record.get_url().empty() && record.get_url().find(id) != std::string::npos)) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<::app::store::payment_links::PaymentLinkRecord>
    find_link_by_payment(std::string_view merchant_id, std::string_view payment_id) const
    {
        for (const auto& record: m_context.repository().list_payment_links()) {
            const bool matches_payment = record.get_link_id() == payment_id ||
                                         (!record.get_url().empty() &&
                                          record.get_url().find(payment_id) != std::string::npos);
            if (matches_payment &&
                (merchant_id.empty() || record.get_merchant_id() == merchant_id)) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<::app::store::payments::PaymentIntentRecord>
    find_intent(std::string_view payment_id) const
    {
        if (auto record = m_context.repository().find_payment_by_string(payment_id)) {
            return record;
        }
        for (const auto& record: m_context.repository().list_payments()) {
            if (record.get_payment_id() == payment_id ||
                record.get_merchant_reference_id() == payment_id) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] ::app::model::payments::RetrievePaymentLinkResponse response_from_link(
        const ::app::store::payment_links::PaymentLinkRecord& record,
        const LinkConfig& config
    ) const
    {
        using Response = ::app::model::payments::RetrievePaymentLinkResponse;
        Response item{};
        item.set_payment_link_id(record.get_link_id());
        ::app::model::payments::PaymentIdValue payment{};
        payment.set_value(record.get_link_id());
        item.set_payment_id(std::move(payment));
        ::app::model::payments::MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(std::move(merchant));
        ::app::model::payments::MerchantIdValue processor{};
        processor.set_value(record.get_merchant_id());
        item.set_processor_merchant_id(std::move(processor));
        if (!record.get_profile_id().empty()) {
            ::app::model::payments::ProfileIdValue profile{};
            profile.set_value(record.get_profile_id());
            item.set_profile_id(std::move(profile));
        }
        item.set_link_to_pay(
            record.get_url().empty() ? std::format(
                                           "https://{}/{}/{}",
                                           config.domain,
                                           record.get_merchant_id(),
                                           record.get_link_id()
                                       )
                                     : record.get_url()
        );
        ::app::model::payments::MinorUnitValue amount{};
        amount.set_value(major_unit_amount(record.get_amount(), record.get_currency()));
        item.set_amount(std::move(amount));
        ::app::model::payments::PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        item.set_created_at(std::move(created));
        if (!record.get_expires_at().empty()) {
            ::app::model::payments::PrimitiveDateTimeValue expiry{};
            expiry.set_value(record.get_expires_at());
            item.set_expiry(std::move(expiry));
        }
        if (!record.get_currency().empty()) {
            ::app::model::payments::CurrencyValue currency{};
            currency.set_value(record.get_currency());
            item.set_currency(std::move(currency));
        }
        item.set_status(
            is_expired(record.get_expires_at()) ? ::app::model::payments::PaymentLinkStatus::Expired
                                                : ::app::model::payments::PaymentLinkStatus::Active
        );
        item.set_secure_link("s/" + record.get_merchant_id() + "/" + record.get_link_id());
        return item;
    }

    [[nodiscard]] ::app::model::payments::RetrievePaymentLinkResponse response_from_intent(
        const ::app::store::payments::PaymentIntentRecord& record,
        const LinkConfig& config
    ) const
    {
        using Response = ::app::model::payments::RetrievePaymentLinkResponse;
        Response item{};
        item.set_payment_link_id(record.get_payment_id());
        ::app::model::payments::PaymentIdValue payment{};
        payment.set_value(record.get_payment_id());
        item.set_payment_id(std::move(payment));
        ::app::model::payments::MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(std::move(merchant));
        item.set_link_to_pay(
            std::format(
                "https://{}/{}/{}",
                config.domain,
                record.get_merchant_id(),
                record.get_payment_id()
            )
        );
        ::app::model::payments::MinorUnitValue amount{};
        amount.set_value(major_unit_amount(record.get_amount(), record.get_currency()));
        item.set_amount(std::move(amount));
        ::app::model::payments::PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        item.set_created_at(std::move(created));
        if (!record.get_currency().empty()) {
            ::app::model::payments::CurrencyValue currency{};
            currency.set_value(record.get_currency());
            item.set_currency(std::move(currency));
        }
        if (!record.get_description().empty() && !unsafe_text(record.get_description())) {
            item.set_description(record.get_description());
        }
        item.set_status(::app::model::payments::PaymentLinkStatus::Active);
        return item;
    }

    /**
     * @brief list_payment_link_by_processor_merchant_id + real total count
     * (core:557-619). Reads the dedicated link table first; payment intents
     * remain a documented fallback for links not mirrored into that table.
     */
    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentLinkListResponse>
        build_list_response(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentLinkListData{dto, ctx};
        std::size_t limit = parse_size(input.request.get_limit().get_value());
        std::size_t offset = parse_size(input.request.get_offset().get_value());
        if (input.query_limit) {
            limit = parse_size(*input.query_limit);
        }
        if (input.query_offset) {
            offset = parse_size(*input.query_offset);
        }
        if (limit == 0) {
            limit = 10U;
        }

        std::optional<std::string> raw_range =
            input.request.get_time_range()
                ? std::optional<std::string>{input.request.get_time_range()->get_value()}
                : std::nullopt;
        if (input.query_time_range) {
            raw_range = *input.query_time_range;
        }
        auto window = time_window(raw_range);
        if (!window) {
            return std::unexpected{window.error()};
        }

        const std::string merchant_filter =
            input.header_merchant_id.empty() ? input.auth_merchant_id : input.header_merchant_id;
        std::string profile_filter{};
        if (input.query_profile_id) {
            profile_filter = *input.query_profile_id;
        } else if (input.profile_scoped) {
            profile_filter = input.auth_profile_id;
        }

        using Response = ::app::model::payments::RetrievePaymentLinkResponse;
        std::vector<Response> matched{};
        const auto links = m_context.repository().list_payment_links();
        const bool use_link_table = !links.empty();
        if (use_link_table) {
            for (const auto& record: links) {
                if (!merchant_filter.empty() && record.get_merchant_id() != merchant_filter) {
                    continue;
                }
                if (!profile_filter.empty() && record.get_profile_id() != profile_filter) {
                    continue;
                }
                if (!within_range(record.get_created_at(), *window)) {
                    continue;
                }
                const auto config = resolve_config(
                    record.get_merchant_id(),
                    record.get_profile_id(),
                    record.get_link_id(),
                    std::string{}
                );
                matched.push_back(response_from_link(record, config));
            }
        } else {
            for (const auto& record: m_context.repository().list_payments()) {
                if (!merchant_filter.empty() && record.get_merchant_id() != merchant_filter) {
                    continue;
                }
                if (!within_range(record.get_created_at(), *window)) {
                    continue;
                }
                const auto config = resolve_config(
                    record.get_merchant_id(),
                    std::string{},
                    record.get_payment_id(),
                    record.get_metadata()
                );
                matched.push_back(response_from_intent(record, config));
            }
        }

        // get_total_count_of_payment_links(): real filtered count before pagination.
        const std::int64_t total_count = static_cast<std::int64_t>(matched.size());
        std::vector<Response> data{};
        data.reserve(std::min(limit, matched.size()));
        for (std::size_t index = offset; index < matched.size() && data.size() < limit; ++index) {
            data.push_back(std::move(matched[index]));
        }

        ::app::model::payments::PaymentLinkListResponse response{};
        response.set_size(data.size());
        response.set_total_count(total_count);
        response.set_data(std::move(data));
        return response;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::payment_link
