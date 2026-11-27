export module app_payment:telephone_order_runtime_common;

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

import :telephone_order_input;

export namespace app::handlers::telephone_order {

// ---------------------------------------------------------------------------
// core/telephone_order.rs error_codes + api_models/telephone_order/twilio.rs
// ---------------------------------------------------------------------------
namespace error_codes {
    inline constexpr std::string_view INVALID_REQUEST{"INVALID_REQUEST"};
    inline constexpr std::string_view PROFILE_NOT_FOUND{"PROFILE_NOT_FOUND"};
    inline constexpr std::string_view PAYMENT_NOT_COMPLETED{"PAYMENT_NOT_COMPLETED"};
    inline constexpr std::string_view PROCESSING_ERROR{"PROCESSING_ERROR"};
    inline constexpr std::string_view DUPLICATE_TRANSACTION{"DUPLICATE_TRANSACTION"};
    inline constexpr std::string_view PAYMENT_PENDING{"PAYMENT_PENDING"};
    inline constexpr std::string_view TOKENIZE_UNSUPPORTED{"TOKENIZE_UNSUPPORTED"};
    inline constexpr std::string_view BANK_ACCOUNT_UNSUPPORTED{"BANK_ACCOUNT_UNSUPPORTED"};
} // namespace error_codes

/** @brief core/telephone_order.rs:41-70 — a provider-readable failure. */
struct Failure
{
    std::string code{};
    std::string message{};
    ResponseParameters parameters{};

    [[nodiscard]] static Failure make(std::string_view code, std::string message)
    {
        return Failure{std::string{code}, std::move(message), {}};
    }

    [[nodiscard]] static Failure processing_error()
    {
        return make(error_codes::PROCESSING_ERROR, "Payment could not be processed");
    }

    [[nodiscard]] Failure with_parameters(ResponseParameters values) &&
    {
        parameters = std::move(values);
        return std::move(*this);
    }
};

enum class OutcomeKind : std::uint8_t
{
    Paid,
    Declined,
    Indeterminate,
};

/** @brief core/telephone_order.rs:74-85 — Outcome::{Paid,Declined,Indeterminate}. */
struct Outcome
{
    OutcomeKind kind{OutcomeKind::Paid};
    Failure failure{};
};

/** @brief core/telephone_order.rs:88-116 — the adapter-independent payment half. */
struct Context
{
    std::optional<std::string> profile_id{};
    std::string payment_id{};
    std::optional<std::string> customer_id{};
    std::optional<std::string> metadata{}; // serialized serde_json::Value
};

/** @brief api_models/telephone_order/twilio.rs:7-14 — `charge` | `tokenize`. */
enum class TwilioPayMethod : std::uint8_t
{
    Charge,
    Tokenize,
};

/** @brief api_models/telephone_order/twilio.rs:71-124 — the provider response body. */
class TwilioGenericPayResponse : public serde::Serializer<TwilioGenericPayResponse>
{
public:
    void set_charge_id(std::optional<std::string> value)
    {
        charge_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_charge_id() const noexcept
    {
        return charge_id;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_parameters(ResponseParameters value)
    {
        parameters = std::move(value);
    }

    [[nodiscard]] const ResponseParameters& get_parameters() const noexcept
    {
        return parameters;
    }

    std::optional<std::string> charge_id{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    ResponseParameters parameters{};
};

/** @brief The mapped PaymentsRequest (core/telephone_order.rs:101-116 + twilio.rs:96-116). */
struct TelephoneOrderPaymentsRequest
{
    std::string payment_id{};
    std::string merchant_id{};
    std::int64_t amount{};
    std::string currency{};
    std::optional<std::string> description{};
    std::optional<std::string> merchant_order_reference_id{};
    std::optional<std::string> card_last4{};
    std::optional<std::string> billing_zip{};
    std::optional<std::string> metadata{};
    std::optional<std::string> profile_id{};
    std::optional<std::string> customer_id{};
    bool confirm{true};
    std::string capture_method{"automatic"};
    std::string authentication_type{"no_three_ds"};
    std::string payment_channel{"telephone_order"};
};

/** @brief Local mirror of the PaymentsResponse fields the core reads
 * (core/telephone_order.rs:286-664). */
struct PaymentView
{
    std::string payment_id{};
    std::string status{}; // canonical upstream IntentStatus
    std::int64_t amount_capturable{};
    std::optional<std::int64_t> amount_received{};
    std::optional<std::string> merchant_order_reference_id{};
    std::optional<std::string> connector{};
    std::optional<std::string> connector_transaction_id{};
    std::optional<std::string> unified_code{};
    std::optional<std::string> unified_message{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
};

/** @brief Local AuthenticationData substitute (services/authentication.rs). */
struct AuthenticationData
{
    std::string merchant_id{};
    std::optional<std::string> profile_id{}; // profile resolved by authentication
};

template<typename T>
struct Result
{
    bool ok{};
    T value{};
    Failure failure{Failure::processing_error()};

    [[nodiscard]] static Result success(T v)
    {
        return Result{true, std::move(v), {}};
    }

    [[nodiscard]] static Result error(Failure f)
    {
        return Result{false, {}, std::move(f)};
    }

    template<typename Function>
    [[nodiscard]] auto and_then(Function&& function) const
    {
        using Next = std::decay_t<decltype(function(std::declval<const T&>()))>;
        if (!ok) {
            return Next::error(failure);
        }
        return function(value);
    }
};

// ---------------------------------------------------------------------------
// Local SHA-256 (common_utils::crypto::Sha256 generate_digest) — exact bytes.
// Self-contained because the shared utils_hash module is not a handler dependency.
// ---------------------------------------------------------------------------
class Sha256Digest
{
public:
    [[nodiscard]] static std::string hex(std::string_view input) noexcept
    {
        static constexpr std::uint32_t k[64]{
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
            0x90'BE'FF'FAU, 0xA4'50'6C'EBU, 0xBE'F9'A3'F7U, 0xC6'71'78'F2U
        };

        std::uint32_t h[8]{
            0x6A'09'E6'67U,
            0xBB'67'AE'85U,
            0x3C'6E'F3'72U,
            0xA5'4F'F5'3AU,
            0x51'0E'52'7FU,
            0x9B'05'68'8CU,
            0x1F'83'D9'ABU,
            0x5B'E0'CD'19U
        };

        std::string message{input};
        const std::uint64_t bit_length = static_cast<std::uint64_t>(input.size()) * 8U;
        message.push_back(static_cast<char>(0x80));
        while (message.size() % 64 != 56) {
            message.push_back(static_cast<char>(0));
        }
        for (int shift = 56; shift >= 0; shift -= 8) {
            message.push_back(static_cast<char>((bit_length >> shift) & 0xFFU));
        }

        for (std::size_t offset = 0; offset < message.size(); offset += 64) {
            std::uint32_t w[64]{};
            for (int i = 0; i < 16; ++i) {
                w[i] = (static_cast<std::uint32_t>(
                            static_cast<unsigned char>(message[offset + 4U * i])
                        )
                        << 24) |
                       (static_cast<std::uint32_t>(
                            static_cast<unsigned char>(message[offset + 4U * i + 1])
                        )
                        << 16) |
                       (static_cast<std::uint32_t>(
                            static_cast<unsigned char>(message[offset + 4U * i + 2])
                        )
                        << 8) |
                       static_cast<std::uint32_t>(
                           static_cast<unsigned char>(message[offset + 4U * i + 3])
                       );
            }
            for (int i = 16; i < 64; ++i) {
                const std::uint32_t s0 =
                    rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
            std::uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
            for (int i = 0; i < 64; ++i) {
                const std::uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                const std::uint32_t ch = (e & f) ^ (~e & g);
                const std::uint32_t t1 = hh + s1 + ch + k[i] + w[i];
                const std::uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                const std::uint32_t t2 = s0 + maj;
                hh = g;
                g = f;
                f = e;
                e = d + t1;
                d = c;
                c = b;
                b = a;
                a = t1 + t2;
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

        static constexpr std::string_view digits{"0123456789abcdef"};
        std::string out{};
        out.reserve(64);
        for (const std::uint32_t word: h) {
            for (int shift = 28; shift >= 0; shift -= 4) {
                out.push_back(digits[(word >> shift) & 0x0FU]);
            }
        }
        return out;
    }

private:
    [[nodiscard]] static std::uint32_t rotr(std::uint32_t value, int bits) noexcept
    {
        return (value >> bits) | (value << (32 - bits));
    }
};

// ---------------------------------------------------------------------------
// Internal JSON helpers for connector payloads and stored metadata. Response bodies
// are typed values serialized by the handler via serde::Ser::serialize.
// ---------------------------------------------------------------------------
inline std::string json_string(std::string_view value)
{
    return serde::Ser::encode_json(std::string{value});
}

inline std::string json_nullable(const std::optional<std::string>& value)
{
    return value ? json_string(*value) : std::string{"null"};
}

inline std::string json_string_map(const std::unordered_map<std::string, std::string>& values)
{
    std::string out{"{"};
    bool first{true};
    for (const auto& [key, value]: values) {
        if (!first) {
            out.push_back(',');
        }
        first = false;
        out += json_string(key);
        out.push_back(':');
        out += json_string(value);
    }
    out.push_back('}');
    return out;
}

// ---------------------------------------------------------------------------
// Local helpers for upstream core functions.
// ---------------------------------------------------------------------------
class TelephoneOrderCore
{
public:
    /** @brief core/telephone_order.rs:426-442 — SHA-256[:16], `pay_` prefix. */
    [[nodiscard]] static std::optional<std::string> derive_payment_id(std::string_view reference)
    {
        const std::string digest = Sha256Digest::hex(reference);
        if (digest.size() < 32) {
            return std::nullopt;
        }
        return "pay_" + digest.substr(0, 32);
    }

    /**
     * @brief core/telephone_order.rs:373-389 — extract one Hyperswitch id from parameters.
     * common_utils id types validate trimmed length [1,64] and [A-Za-z0-9_-] only
     * (common_utils/src/id_type.rs:74-86, macros.rs:150-169, consts.rs:109-118).
     */
    [[nodiscard]] static Result<std::optional<std::string>>
    extract_id(const Parameters& parameters, std::string_view key)
    {
        const auto it = parameters.find(std::string{key});
        if (it == parameters.end()) {
            return Result<std::optional<std::string>>::success(std::nullopt);
        }

        std::string_view value{it->second};
        while (!value.empty() && is_space(value.front())) {
            value.remove_prefix(1);
        }
        while (!value.empty() && is_space(value.back())) {
            value.remove_suffix(1);
        }
        bool valid = value.size() >= 1 && value.size() <= 64;
        if (valid) {
            for (const char character: value) {
                if (!is_id_char(character)) {
                    valid = false;
                    break;
                }
            }
        }
        if (!valid) {
            return Result<std::optional<std::string>>::error(
                Failure::make(
                    error_codes::INVALID_REQUEST,
                    std::format(
                        "parameters.{} is not a valid {}: Incorrect value provided for field: {}",
                        key,
                        key,
                        key
                    )
                )
            );
        }
        return Result<std::optional<std::string>>::success(std::string{value});
    }

    /** @brief core/telephone_order.rs:185-191 — parameters.payment_id wins, else derive. */
    [[nodiscard]] static Result<std::optional<std::string>>
    resolve_payment_id(const TwilioGenericPayRequest& request)
    {
        const Parameters empty{};
        const Parameters& parameters = request.parameters.value_or(empty);
        auto supplied = extract_id(parameters, "payment_id");
        if (!supplied.ok) {
            return Result<std::optional<std::string>>::error(std::move(supplied.failure));
        }
        if (supplied.value) {
            return Result<std::optional<std::string>>::success(std::move(supplied.value));
        }
        if (request.transaction_id.empty()) {
            return Result<std::optional<std::string>>::error(
                Failure::make(error_codes::INVALID_REQUEST, "transaction_id is required")
            );
        }
        return Result<std::optional<std::string>>::success(
            derive_payment_id(request.transaction_id)
        );
    }

    /** @brief core/telephone_order.rs:393-401 — parameters serialized directly, None when empty. */
    [[nodiscard]] static std::optional<std::string> build_metadata(const Parameters& parameters)
    {
        if (parameters.empty()) {
            return std::nullopt;
        }
        return json_string_map(parameters);
    }

    /**
     * @brief core/telephone_order.rs:358-370 + classify_core_error:510-519 — the profile
     * authentication resolved; a foreign profile is PROFILE_NOT_FOUND.
     */
    [[nodiscard]] static Result<std::optional<std::string>> resolve_profile_id(
        ::app::context::AppContext& context,
        const AuthenticationData& auth,
        const Parameters& parameters
    )
    {
        auto extracted = extract_id(parameters, "profile_id");
        if (!extracted.ok) {
            return Result<std::optional<std::string>>::error(std::move(extracted.failure));
        }
        if (!extracted.value) {
            return Result<std::optional<std::string>>::success(auth.profile_id);
        }
        const std::string& profile_id = *extracted.value;
        const auto record = context.repository().find_profile_by_string(profile_id);
        if (record && !record->get_merchant_id().empty() &&
            record->get_merchant_id() != auth.merchant_id) {
            return Result<std::optional<std::string>>::error(
                Failure::make(
                    error_codes::PROFILE_NOT_FOUND,
                    std::format("profile_id {} is not available for this merchant", profile_id)
                )
            );
        }
        return Result<std::optional<std::string>>::success(profile_id);
    }

    /** @brief core/telephone_order.rs:336-355 — build Context; unusable customer_id is dropped. */
    [[nodiscard]] static Result<Context> build_context(
        ::app::context::AppContext& context,
        const AuthenticationData& auth,
        const Parameters& parameters,
        std::string payment_id
    )
    {
        auto profile = resolve_profile_id(context, auth, parameters);
        if (!profile.ok) {
            return Result<Context>::error(std::move(profile.failure));
        }
        Context result{};
        result.profile_id = std::move(profile.value);
        result.payment_id = std::move(payment_id);
        auto customer = extract_id(parameters, "customer_id");
        if (customer.ok && customer.value) {
            result.customer_id = std::move(customer.value);
        }
        result.metadata = build_metadata(parameters);
        return Result<Context>::success(std::move(result));
    }

    /** @brief core/telephone_order.rs:101-116 — shared PaymentsRequest flags. */
    [[nodiscard]] static TelephoneOrderPaymentsRequest payments_request(const Context& context)
    {
        TelephoneOrderPaymentsRequest request{};
        request.payment_id = context.payment_id;
        request.profile_id = context.profile_id;
        request.customer_id = context.customer_id;
        request.metadata = context.metadata;
        return request;
    }

    /**
     * @brief core/telephone_order.rs:503-559 — classify a payments-core error as a Failure.
     * The local core surfaces errors through AppErrorKind; the message retains the field the
     * upstream variant names so the provider can re-prompt for exactly that input.
     */
    [[nodiscard]] static Failure
    classify_core_error(core::error::AppErrorKind kind, std::string_view message)
    {
        using core::error::AppErrorKind;
        switch (kind) {
            case AppErrorKind::NOT_FOUND:
                return Failure::make(error_codes::PROFILE_NOT_FOUND, std::string{message});
            case AppErrorKind::PRECONDITION_FAILED:
            case AppErrorKind::INVALID_REQUEST:
            case AppErrorKind::VALIDATION_FAILED:
            case AppErrorKind::INVALID_JSON:
            case AppErrorKind::EMPTY_REQUEST:
                return Failure::make(error_codes::INVALID_REQUEST, std::string{message});
            default:
                return Failure::processing_error();
        }
    }

    /** @brief core/telephone_order.rs:563-601 — exhaustive IntentStatus to Outcome mapping. */
    [[nodiscard]] static Outcome payment_outcome(const PaymentView& payment)
    {
        const std::string_view status{payment.status};
        if (status == "succeeded" || status == "partially_captured" ||
            status == "requires_capture" || status == "partially_authorized_and_requires_capture") {
            return Outcome{OutcomeKind::Paid, {}};
        }
        if (status.empty() || status == "processing" ||
            status == "partially_captured_and_capturable" ||
            status == "partially_captured_and_processing" || status == "review" ||
            status == "requires_merchant_action" || status == "conflicted" ||
            status == "cancelled_post_capture") {
            Failure failure = Failure::make(
                error_codes::PAYMENT_PENDING,
                std::format("Payment outcome is not final and funds may be held: {}", status)
            );
            failure.parameters = outcome_parameters(payment);
            return Outcome{OutcomeKind::Indeterminate, std::move(failure)};
        }
        return Outcome{OutcomeKind::Declined, decline_failure(payment)};
    }

    /** @brief core/telephone_order.rs:604-619 — decline in connector terms where available. */
    [[nodiscard]] static Failure decline_failure(const PaymentView& payment)
    {
        std::string code = payment.unified_code.value_or(
            payment.error_code.value_or(std::string{error_codes::PAYMENT_NOT_COMPLETED})
        );
        if (code.empty()) {
            code = std::string{error_codes::PAYMENT_NOT_COMPLETED};
        }
        std::string message = payment.unified_message.value_or(payment.error_message.value_or(
            std::format("Payment was not completed: {}", payment.status)
        ));
        if (message.empty()) {
            message = std::format("Payment was not completed: {}", payment.status);
        }
        Failure failure = Failure::make(code, std::move(message));
        failure.parameters = outcome_parameters(payment);
        return failure;
    }

    /** @brief core/telephone_order.rs:621-664 — explicit all-string parameter list. */
    [[nodiscard]] static ResponseParameters outcome_parameters(const PaymentView& payment)
    {
        ResponseParameters result{
            {"payment_id", payment.payment_id},
            {"status", payment.status},
            {"amount_capturable", std::to_string(payment.amount_capturable)},
        };
        const std::pair<std::string_view, std::optional<std::string>> optional[]{
            {"merchant_order_reference_id", payment.merchant_order_reference_id},
            {"connector", payment.connector},
            {"connector_transaction_id", payment.connector_transaction_id},
            {"unified_code", payment.unified_code},
            {"amount_received",
             payment.amount_received
                 ? std::optional<std::string>{std::to_string(*payment.amount_received)}
                 : std::nullopt},
        };
        for (const auto& [key, value]: optional) {
            if (value) {
                result.emplace(std::string{key}, *value);
            }
        }
        return result;
    }

    /** @brief core/telephone_order.rs:404-421 — validate then settle the payment id. */
    [[nodiscard]] static Result<TelephoneOrderPaymentsRequest> finalize_payments_request(
        TelephoneOrderPaymentsRequest request,
        ::app::crypto::IdGenerator& ids
    )
    {
        if (request.amount <= 0) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(error_codes::INVALID_REQUEST, "amount should be greater than zero")
            );
        }
        if (request.payment_id.empty()) {
            request.payment_id = ids.payment_id();
        }
        return Result<TelephoneOrderPaymentsRequest>::success(std::move(request));
    }

    /** @brief Map a local connector status onto the canonical upstream IntentStatus. */
    [[nodiscard]] static std::string canonical_status(std::string_view raw)
    {
        if (raw == "authorized" || raw == "authorized_ok" || raw == "requires_capture") {
            return "requires_capture";
        }
        if (raw == "captured" || raw == "succeeded" || raw == "success") {
            return "succeeded";
        }
        if (raw == "declined" || raw == "failed" || raw == "failure") {
            return "failed";
        }
        if (raw == "pending" || raw == "processing") {
            return "processing";
        }
        return std::string{raw};
    }

    /**
     * @brief twilio.rs:72-83 — major-unit decimal string to minor units. rust_decimal `to_i64`
     * truncates toward zero; zero/three-decimal currencies use scale 0/3.
     */
    [[nodiscard]] static std::optional<std::int64_t>
    parse_major_unit(std::string_view text, std::string_view currency)
    {
        const int scale = minor_digits(currency);
        std::string digits{};
        bool negative{false};
        bool seen_digit{false};
        bool seen_dot{false};
        int fraction_digits{0};
        for (std::size_t i = 0; i < text.size(); ++i) {
            const char character = text[i];
            if (i == 0 && (character == '+' || character == '-')) {
                negative = character == '-';
                continue;
            }
            if (character == '.' && !seen_dot) {
                seen_dot = true;
                continue;
            }
            if (!std::isdigit(static_cast<unsigned char>(character))) {
                return std::nullopt;
            }
            seen_digit = true;
            if (seen_dot) {
                ++fraction_digits;
            }
            digits.push_back(character);
        }
        if (!seen_digit) {
            return std::nullopt;
        }

        std::string scaled{};
        if (fraction_digits >= scale) {
            scaled =
                digits.substr(0, digits.size() - static_cast<std::size_t>(fraction_digits - scale));
        } else {
            scaled = digits + std::string(static_cast<std::size_t>(scale - fraction_digits), '0');
        }
        std::int64_t amount{};
        for (const char digit: scaled) {
            amount = amount * 10 + (digit - '0');
        }
        return negative ? -amount : amount;
    }

    [[nodiscard]] static bool is_currency(std::string_view value)
    {
        if (value.size() != 3) {
            return false;
        }
        for (const char character: value) {
            if (!std::isupper(static_cast<unsigned char>(character))) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] static std::string normalize_currency(std::string_view value)
    {
        std::string result{};
        for (const char character: value) {
            result.push_back(
                static_cast<char>(std::toupper(static_cast<unsigned char>(character)))
            );
        }
        return result;
    }

    /** @brief core/telephone_order.rs:315-332 — Hold keyed on the resolved payment id. */
    struct LockAction
    {
        bool applicable{};
        std::string key{};
    };

    [[nodiscard]] static LockAction locking_action(const std::optional<std::string>& payment_id)
    {
        if (!payment_id) {
            return LockAction{false, {}};
        }
        return LockAction{true, *payment_id};
    }

private:
    [[nodiscard]] static bool is_space(char character) noexcept
    {
        return std::isspace(static_cast<unsigned char>(character)) != 0;
    }

    [[nodiscard]] static bool is_id_char(char character) noexcept
    {
        const auto byte = static_cast<unsigned char>(character);
        return std::isalnum(byte) != 0 || character == '_' || character == '-';
    }

    [[nodiscard]] static int minor_digits(std::string_view currency) noexcept
    {
        if (currency == "JPY" || currency == "KRW" || currency == "VND" || currency == "CLP") {
            return 0;
        }
        if (currency == "BHD" || currency == "JOD" || currency == "KWD" || currency == "OMR" ||
            currency == "TND") {
            return 3;
        }
        return 2;
    }
};

// ---------------------------------------------------------------------------
// core/telephone_order/twilio.rs — TwilioGenericPay adapter.
// ---------------------------------------------------------------------------
class TwilioGenericPay
{
public:
    /** @brief twilio.rs:34-42 — transaction id is both the log reference and idempotency key. */
    [[nodiscard]] static std::string transaction_reference(const TwilioGenericPayRequest& request)
    {
        return request.transaction_id;
    }

    [[nodiscard]] static std::string_view
    idempotency_reference(const TwilioGenericPayRequest& request)
    {
        return request.transaction_id;
    }

    [[nodiscard]] static Parameters parameters(const TwilioGenericPayRequest& request)
    {
        static const Parameters empty{};
        return request.parameters.value_or(empty);
    }

    /** @brief twilio.rs:135-153 — extract_card with required-field messages. */
    [[nodiscard]] static Result<TelephoneOrderPaymentsRequest>
    extract_card(const TwilioGenericPayRequest& request, TelephoneOrderPaymentsRequest& target)
    {
        const auto missing = [](std::string_view field)
        {
            return Failure::make(
                error_codes::INVALID_REQUEST,
                std::format("{} is required for a card charge", field)
            );
        };
        if (!request.cardnumber || request.cardnumber->empty()) {
            return Result<TelephoneOrderPaymentsRequest>::error(missing("cardnumber"));
        }
        if (!request.expiry_month || request.expiry_month->empty()) {
            return Result<TelephoneOrderPaymentsRequest>::error(missing("expiry_month"));
        }
        if (!request.expiry_year || request.expiry_year->empty()) {
            return Result<TelephoneOrderPaymentsRequest>::error(missing("expiry_year"));
        }
        if (!request.cvv || request.cvv->empty()) {
            return Result<TelephoneOrderPaymentsRequest>::error(missing("cvv"));
        }
        const std::string& number = *request.cardnumber;
        target.card_last4 = number.size() > 4 ? number.substr(number.size() - 4) : number;
        return Result<TelephoneOrderPaymentsRequest>::success(std::move(target));
    }

    /** @brief twilio.rs:49-117 — map the <Pay> fields onto a PaymentsRequest. */
    [[nodiscard]] static Result<TelephoneOrderPaymentsRequest>
    build_payments_request(const TwilioGenericPayRequest& request, const Context& context)
    {
        if (request.method != "charge") {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(
                    error_codes::TOKENIZE_UNSUPPORTED,
                    "Tokenization is not supported on this endpoint"
                )
            );
        }
        if (request.bankaccountnumber || request.routingnumber) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(
                    error_codes::BANK_ACCOUNT_UNSUPPORTED,
                    "Bank account payments are not supported on this endpoint"
                )
            );
        }

        TelephoneOrderPaymentsRequest payment = TelephoneOrderCore::payments_request(context);
        auto card = extract_card(request, payment);
        if (!card.ok) {
            return Result<TelephoneOrderPaymentsRequest>::error(std::move(card.failure));
        }
        payment = std::move(card.value);

        if (!request.currency_code) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(error_codes::INVALID_REQUEST, "currency_code is required")
            );
        }
        const std::string currency = TelephoneOrderCore::normalize_currency(*request.currency_code);
        if (!TelephoneOrderCore::is_currency(currency)) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(error_codes::INVALID_REQUEST, "currency_code is required")
            );
        }
        if (!request.amount) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(error_codes::INVALID_REQUEST, "amount is required")
            );
        }
        const auto minor = TelephoneOrderCore::parse_major_unit(*request.amount, currency);
        if (!minor) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(
                    error_codes::INVALID_REQUEST,
                    std::format("amount could not be parsed: {}", *request.amount)
                )
            );
        }

        payment.amount = *minor;
        payment.currency = currency;
        payment.description = request.description;
        payment.merchant_order_reference_id = request.transaction_id;
        payment.billing_zip = request.postal_code;
        return Result<TelephoneOrderPaymentsRequest>::success(std::move(payment));
    }

    /** @brief twilio.rs:119-128 — success/failure response constructors. */
    [[nodiscard]] static TwilioGenericPayResponse
    success_response(const PaymentView& payment, ResponseParameters response_parameters)
    {
        TwilioGenericPayResponse response{};
        response.set_charge_id(payment.payment_id);
        response.set_parameters(std::move(response_parameters));
        return response;
    }

    [[nodiscard]] static TwilioGenericPayResponse failure_response(Failure failure)
    {
        TwilioGenericPayResponse response{};
        response.set_error_code(std::move(failure.code));
        response.set_error_message(std::move(failure.message));
        response.set_parameters(std::move(failure.parameters));
        return response;
    }

    /** @brief core/telephone_order.rs:193-205 — log + failure response. */
    [[nodiscard]] static TwilioGenericPayResponse report_failure(
        ::app::context::AppContext& context,
        const std::string& transaction_reference,
        Failure failure,
        std::string_view flow_name
    )
    {
        context.metrics().counter(std::string{flow_name} + "_failure").increment();
        (void)transaction_reference;
        return failure_response(std::move(failure));
    }
};

/** @brief Auth verification + profile resolution used by the charge sub-runtime. */
class TelephoneOrderRuntimeSupport
{
public:
    explicit TelephoneOrderRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief Upstream code in `error`, human-readable text in `message`. */
    [[nodiscard]] core::error::AppError
    api_error(core::error::AppErrorKind kind, std::string message, std::string_view code) const
    {
        auto error = core::error::AppErrorBuilder{}(kind, std::move(message));
        error.set_kind_name(std::string{code});
        return error;
    }

    /** @brief services/authentication.rs — MERCHANT_ID / PAYMENT_WRITE with api-key claim. */
    [[nodiscard]] bool authorize(const ::app::auth::AuthContext& auth_context) const
    {
        const ::app::auth::RouteAuthRule rule =
            ::app::auth::RouteAuthRuleBuilder{}
                .scheme(::app::auth::AuthScheme::MERCHANT_ID)
                .permission(::app::auth::AppPermission::PAYMENT_WRITE)
                .id_source(std::string{::app::auth::kClaimApiKey})
                .build();
        return m_context.authorizer().authorize_route(auth_context, rule).has_value();
    }

    /**
     * @brief authenticator() hands extract_id(parameters,"profile_id").ok().flatten() to auth;
     * charge() re-parses it and rejects an unusable id there.
     */
    template<class Dto>
    [[nodiscard]] AuthenticationData
        authentication_data(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = TelephoneOrderChargeData{dto, ctx};
        AuthenticationData auth{};
        auth.merchant_id = input.merchant_id;
        auto profile = TelephoneOrderCore::extract_id(
            TwilioGenericPay::parameters(input.request),
            "profile_id"
        );
        if (profile.ok && profile.value) {
            auth.profile_id = std::move(profile.value);
        } else {
            const auto account =
                m_context.repository().find_merchant_account_by_string(input.merchant_id);
            if (account && !account->get_default_profile().empty()) {
                auth.profile_id = account->get_default_profile();
            }
        }
        return auth;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::telephone_order

template<>
struct serde::Serializable<::app::handlers::telephone_order::TwilioGenericPayResponse>
{
    static constexpr auto fields()
    {
        using Wire = ::app::handlers::telephone_order::TwilioGenericPayResponse;
        return std::tuple{
            serde::FieldDesc<
                "charge_id",
                &Wire::get_charge_id,
                &Wire::set_charge_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &Wire::get_error_code,
                &Wire::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &Wire::get_error_message,
                &Wire::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parameters",
                &Wire::get_parameters,
                &Wire::set_parameters,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "twilio_generic_pay_response";
    }
};
