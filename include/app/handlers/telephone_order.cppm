module;
export module app_handlers:telephone_order;

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
import app_store;
import app_connector;
import app_http;
import app_metrics;
import app_crypto;
import app_locking;

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

using Parameters = std::unordered_map<std::string, std::string>;
using ResponseParameters = std::unordered_map<std::string, std::string>;

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

/** @brief api_models/telephone_order/twilio.rs:16-69 — the provider request body. */
class TwilioGenericPayRequest
{
public:
    void set_method(std::string value) { method = std::move(value); }
    [[nodiscard]] const std::string& get_method() const noexcept { return method; }
    void set_transaction_id(std::string value) { transaction_id = std::move(value); }
    [[nodiscard]] const std::string& get_transaction_id() const noexcept { return transaction_id; }
    void set_cardnumber(std::optional<std::string> value) { cardnumber = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_cardnumber() const noexcept { return cardnumber; }
    void set_cvv(std::optional<std::string> value) { cvv = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_cvv() const noexcept { return cvv; }
    void set_expiry_month(std::optional<std::string> value) { expiry_month = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_expiry_month() const noexcept { return expiry_month; }
    void set_expiry_year(std::optional<std::string> value) { expiry_year = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_expiry_year() const noexcept { return expiry_year; }
    void set_postal_code(std::optional<std::string> value) { postal_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_postal_code() const noexcept { return postal_code; }
    void set_bankaccountnumber(std::optional<std::string> value) { bankaccountnumber = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_bankaccountnumber() const noexcept { return bankaccountnumber; }
    void set_routingnumber(std::optional<std::string> value) { routingnumber = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_routingnumber() const noexcept { return routingnumber; }
    void set_amount(std::optional<std::string> value) { amount = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_amount() const noexcept { return amount; }
    void set_currency_code(std::optional<std::string> value) { currency_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_currency_code() const noexcept { return currency_code; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_parameters(std::optional<Parameters> value) { parameters = std::move(value); }
    [[nodiscard]] const std::optional<Parameters>& get_parameters() const noexcept { return parameters; }

    std::string method{};
    std::string transaction_id{};
    std::optional<std::string> cardnumber{};
    std::optional<std::string> cvv{};
    std::optional<std::string> expiry_month{};
    std::optional<std::string> expiry_year{};
    std::optional<std::string> postal_code{};
    std::optional<std::string> bankaccountnumber{};
    std::optional<std::string> routingnumber{};
    std::optional<std::string> amount{};
    std::optional<std::string> currency_code{};
    std::optional<std::string> description{};
    std::optional<Parameters> parameters{};
};

/** @brief api_models/telephone_order/twilio.rs:71-124 — the provider response body. */
class TwilioGenericPayResponse
{
public:
    void set_charge_id(std::optional<std::string> value) { charge_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_charge_id() const noexcept { return charge_id; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_parameters(ResponseParameters value) { parameters = std::move(value); }
    [[nodiscard]] const ResponseParameters& get_parameters() const noexcept { return parameters; }

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

/** @brief Local mirror of the PaymentsResponse fields the core reads (core/telephone_order.rs:286-664). */
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

    [[nodiscard]] static Result success(T v) { return Result{true, std::move(v), {}}; }
    [[nodiscard]] static Result error(Failure f) { return Result{false, {}, std::move(f)}; }

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
            0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU, 0x59f111f1U,
            0x923f82a4U, 0xab1c5ed5U, 0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
            0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U, 0xe49b69c1U, 0xefbe4786U,
            0x0fc19dc6U, 0x240ca1ccU, 0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
            0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U,
            0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
            0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U, 0xa2bfe8a1U, 0xa81a664bU,
            0xc24b8b70U, 0xc76c51a3U, 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
            0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU,
            0x5b9cca4fU, 0x682e6ff3U, 0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
            0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U};

        std::uint32_t h[8]{0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
                           0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};

        std::string message{input};
        const std::uint64_t bit_length = static_cast<std::uint64_t>(input.size()) * 8U;
        message.push_back(static_cast<char>(0x80));
        while (message.size() % 64 != 56) {
            message.push_back(static_cast<char>(0));
        }
        for (int shift = 56; shift >= 0; shift -= 8) {
            message.push_back(static_cast<char>((bit_length >> shift) & 0xffU));
        }

        for (std::size_t offset = 0; offset < message.size(); offset += 64) {
            std::uint32_t w[64]{};
            for (int i = 0; i < 16; ++i) {
                w[i] = (static_cast<std::uint32_t>(static_cast<unsigned char>(message[offset + 4U * i])) << 24) |
                       (static_cast<std::uint32_t>(static_cast<unsigned char>(message[offset + 4U * i + 1])) << 16) |
                       (static_cast<std::uint32_t>(static_cast<unsigned char>(message[offset + 4U * i + 2])) << 8) |
                       static_cast<std::uint32_t>(static_cast<unsigned char>(message[offset + 4U * i + 3]));
            }
            for (int i = 16; i < 64; ++i) {
                const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
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
                hh = g; g = f; f = e; e = d + t1;
                d = c; c = b; b = a; a = t1 + t2;
            }
            h[0] += a; h[1] += b; h[2] += c; h[3] += d;
            h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
        }

        static constexpr std::string_view digits{"0123456789abcdef"};
        std::string out{};
        out.reserve(64);
        for (const std::uint32_t word : h) {
            for (int shift = 28; shift >= 0; shift -= 4) {
                out.push_back(digits[(word >> shift) & 0x0fU]);
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
// JSON wire helpers — emit exact upstream shapes (all fields present; `null` for None).
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
    for (const auto& [key, value] : values) {
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

inline std::string encode_response(const TwilioGenericPayResponse& response)
{
    std::string out{"{"};
    out += R"("charge_id":)" + json_nullable(response.charge_id);
    out += R"(,"error_code":)" + json_nullable(response.error_code);
    out += R"(,"error_message":)" + json_nullable(response.error_message);
    out += R"(,"parameters":)" + json_string_map(response.parameters);
    out.push_back('}');
    return out;
}

/** @brief api_models/errors/types.rs:39-70 — `{"type","message","code"}`. */
inline std::string encode_api_error(
    std::string_view type, std::string_view message, std::string_view code)
{
    return std::string{R"({"type":)"} + json_string(type) + R"(,"message":)" +
           json_string(message) + R"(,"code":)" + json_string(code) + "}";
}

// ---------------------------------------------------------------------------
// Local helpers for upstream core functions.
// ---------------------------------------------------------------------------
class TelephoneOrderCore
{
public:
    /** @brief core/telephone_order.rs:426-442 — SHA-256[:16], `pay_` prefix. */
    [[nodiscard]] static std::optional<std::string>
    derive_payment_id(std::string_view reference)
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
            for (const char character : value) {
                if (!is_id_char(character)) {
                    valid = false;
                    break;
                }
            }
        }
        if (!valid) {
            return Result<std::optional<std::string>>::error(Failure::make(
                error_codes::INVALID_REQUEST,
                std::format("parameters.{} is not a valid {}: Incorrect value provided for field: {}",
                            key, key, key)));
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
                Failure::make(error_codes::INVALID_REQUEST, "transaction_id is required"));
        }
        return Result<std::optional<std::string>>::success(derive_payment_id(request.transaction_id));
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
        app::context::AppContext& context,
        const AuthenticationData& auth,
        const Parameters& parameters)
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
            return Result<std::optional<std::string>>::error(Failure::make(
                error_codes::PROFILE_NOT_FOUND,
                std::format("profile_id {} is not available for this merchant", profile_id)));
        }
        return Result<std::optional<std::string>>::success(profile_id);
    }

    /** @brief core/telephone_order.rs:336-355 — build Context; unusable customer_id is dropped. */
    [[nodiscard]] static Result<Context> build_context(
        app::context::AppContext& context,
        const AuthenticationData& auth,
        const Parameters& parameters,
        std::string payment_id)
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
    [[nodiscard]] static Failure classify_core_error(
        core::error::AppErrorKind kind, std::string_view message)
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
                std::format("Payment outcome is not final and funds may be held: {}", status));
            failure.parameters = outcome_parameters(payment);
            return Outcome{OutcomeKind::Indeterminate, std::move(failure)};
        }
        return Outcome{OutcomeKind::Declined, decline_failure(payment)};
    }

    /** @brief core/telephone_order.rs:604-619 — decline in connector terms where available. */
    [[nodiscard]] static Failure decline_failure(const PaymentView& payment)
    {
        std::string code = payment.unified_code.value_or(
            payment.error_code.value_or(std::string{error_codes::PAYMENT_NOT_COMPLETED}));
        if (code.empty()) {
            code = std::string{error_codes::PAYMENT_NOT_COMPLETED};
        }
        std::string message = payment.unified_message.value_or(
            payment.error_message.value_or(
                std::format("Payment was not completed: {}", payment.status)));
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
             payment.amount_received ? std::optional<std::string>{
                                           std::to_string(*payment.amount_received)}
                                     : std::nullopt},
        };
        for (const auto& [key, value] : optional) {
            if (value) {
                result.emplace(std::string{key}, *value);
            }
        }
        return result;
    }

    /** @brief core/telephone_order.rs:404-421 — validate then settle the payment id. */
    [[nodiscard]] static Result<TelephoneOrderPaymentsRequest>
    finalize_payments_request(TelephoneOrderPaymentsRequest request, app::crypto::IdGenerator& ids)
    {
        if (request.amount <= 0) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(error_codes::INVALID_REQUEST, "amount should be greater than zero"));
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
            scaled = digits.substr(
                0, digits.size() - static_cast<std::size_t>(fraction_digits - scale));
        } else {
            scaled = digits + std::string(static_cast<std::size_t>(scale - fraction_digits), '0');
        }
        std::int64_t amount{};
        for (const char digit : scaled) {
            amount = amount * 10 + (digit - '0');
        }
        return negative ? -amount : amount;
    }

    [[nodiscard]] static bool is_currency(std::string_view value)
    {
        if (value.size() != 3) {
            return false;
        }
        for (const char character : value) {
            if (!std::isupper(static_cast<unsigned char>(character))) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] static std::string normalize_currency(std::string_view value)
    {
        std::string result{};
        for (const char character : value) {
            result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(character))));
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

    [[nodiscard]] static std::string_view idempotency_reference(const TwilioGenericPayRequest& request)
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
                std::format("{} is required for a card charge", field));
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
            return Result<TelephoneOrderPaymentsRequest>::error(Failure::make(
                error_codes::TOKENIZE_UNSUPPORTED,
                "Tokenization is not supported on this endpoint"));
        }
        if (request.bankaccountnumber || request.routingnumber) {
            return Result<TelephoneOrderPaymentsRequest>::error(Failure::make(
                error_codes::BANK_ACCOUNT_UNSUPPORTED,
                "Bank account payments are not supported on this endpoint"));
        }

        TelephoneOrderPaymentsRequest payment = TelephoneOrderCore::payments_request(context);
        auto card = extract_card(request, payment);
        if (!card.ok) {
            return Result<TelephoneOrderPaymentsRequest>::error(std::move(card.failure));
        }
        payment = std::move(card.value);

        if (!request.currency_code) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(error_codes::INVALID_REQUEST, "currency_code is required"));
        }
        const std::string currency = TelephoneOrderCore::normalize_currency(*request.currency_code);
        if (!TelephoneOrderCore::is_currency(currency)) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(error_codes::INVALID_REQUEST, "currency_code is required"));
        }
        if (!request.amount) {
            return Result<TelephoneOrderPaymentsRequest>::error(
                Failure::make(error_codes::INVALID_REQUEST, "amount is required"));
        }
        const auto minor = TelephoneOrderCore::parse_major_unit(*request.amount, currency);
        if (!minor) {
            return Result<TelephoneOrderPaymentsRequest>::error(Failure::make(
                error_codes::INVALID_REQUEST,
                std::format("amount could not be parsed: {}", *request.amount)));
        }

        payment.amount = *minor;
        payment.currency = currency;
        payment.description = request.description;
        payment.merchant_order_reference_id = request.transaction_id;
        payment.billing_zip = request.postal_code;
        return Result<TelephoneOrderPaymentsRequest>::success(std::move(payment));
    }

    /** @brief twilio.rs:119-128 — success/failure response constructors. */
    [[nodiscard]] static TwilioGenericPayResponse success_response(
        const PaymentView& payment, ResponseParameters response_parameters)
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
        app::context::AppContext& context,
        const std::string& transaction_reference,
        Failure failure,
        std::string_view flow_name)
    {
        context.metrics().counter(std::string{flow_name} + "_failure").increment();
        (void)transaction_reference;
        return failure_response(std::move(failure));
    }
};

// ---------------------------------------------------------------------------
// Route-level helpers: Basic auth decoding.
// ---------------------------------------------------------------------------
inline std::string decode_base64(std::string_view text)
{
    static constexpr std::string_view table{
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
    std::string out{};
    std::uint32_t accumulator{};
    int bits{};
    for (const char character : text) {
        if (character == '=' || std::isspace(static_cast<unsigned char>(character))) {
            continue;
        }
        const std::size_t value = table.find(character);
        if (value == std::string_view::npos) {
            return {};
        }
        accumulator = (accumulator << 6) | static_cast<std::uint32_t>(value);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<char>((accumulator >> bits) & 0xffU));
        }
    }
    return out;
}

inline std::optional<std::string> basic_api_key(interfaces::io::IRequest& req)
{
    const std::string_view authorization = req.find_header("authorization");
    constexpr std::string_view prefix{"Basic "};
    if (authorization.size() > prefix.size() &&
        std::equal(prefix.begin(), prefix.end(), authorization.begin(),
                   [](char lhs, char rhs)
                   {
                       return std::tolower(static_cast<unsigned char>(lhs)) ==
                              std::tolower(static_cast<unsigned char>(rhs));
                   })) {
        const std::string decoded = decode_base64(authorization.substr(prefix.size()));
        const std::size_t colon = decoded.find(':');
        const std::string key = colon == std::string::npos ? decoded : decoded.substr(0, colon);
        if (!key.empty()) {
            return key;
        }
    }
    const std::string_view header = req.find_header("x-api-key");
    if (!header.empty()) {
        return std::string{header};
    }
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// Handler / route surface.
// ---------------------------------------------------------------------------
class TelephoneOrderRouteHandlers
{
public:
    /**
     * @brief routes/telephone_order.rs:19-36 — telephone_order_charge.
     * Routes to the Twilio adapter; an unknown provider is InvalidRequestUrl.
     */
    void handle_telephone_order_charge(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        const auto merchant_segment = count >= 2 ? segments.at(count - 2) : std::nullopt;
        const auto provider_segment = count >= 1 ? segments.at(count - 1) : std::nullopt;
        const std::string merchant_id = merchant_segment ? std::string{*merchant_segment} : std::string{};
        const std::string provider = provider_segment ? std::string{*provider_segment} : std::string{};

        if (merchant_id.empty() || provider != "twilio") {
            m_context->metrics().counter("telephone_order_charge_invalid_request_url").increment();
            send_api_error(
                res, send,
                encode_api_error("invalid_request", "Unrecognized request URL", "IR_02"),
                interfaces::io::types::Status::NOT_FOUND);
            return;
        }

        charge_with<TwilioGenericPay>(req, res, std::move(send), merchant_id);
    }

    /** @brief routes/telephone_order.rs:38-91 — charge_with<A>. */
    template<typename Adapter>
    void charge_with(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        const std::string& merchant_id)
    {
        const std::string body = req.body_as_string();
        auto parsed = serde::Ser::deserialize<TwilioGenericPayRequest>("application/json", body);
        // An unknown TwilioPayMethod is a deserialization failure upstream (enum variant).
        if (parsed && parsed->get_method() != "charge" && parsed->get_method() != "tokenize") {
            parsed = std::unexpected{std::string{"Unknown TwilioPayMethod"}};
        }
        if (!parsed) {
            m_context->metrics().counter("telephone_order_charge_invalid_data_format").increment();
            send_api_error(
                res, send,
                encode_api_error(
                    "invalid_request",
                    "request body contains invalid data. Expected format is a telephone order charge request for this provider",
                    "IR_05"),
                interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        const auto& request = *parsed;
        const std::string transaction_reference = Adapter::transaction_reference(request);

        m_context->metrics().record_flow("TelephoneOrderCharge");
        m_context->metrics().counter("telephone_order_charge_total").increment();

        // The route resolves the id once (routes/telephone_order.rs:52-62) — span + lock.
        auto resolved_id = TelephoneOrderCore::resolve_payment_id(request);
        std::optional<std::string> payment_id{};
        if (resolved_id.ok) {
            payment_id = std::move(resolved_id.value);
        }
        if (payment_id) {
            m_context->metrics().counter("telephone_order_charge_payment_id_resolved").increment();
        }

        const auto action = TelephoneOrderCore::locking_action(payment_id);
        std::optional<app::locking::ApiLock> lock{};
        if (action.applicable) {
            lock = m_context->locks().acquire(action.key, std::int64_t{30}, "telephone_order_charge");
            if (!lock) {
                m_context->metrics().counter("telephone_order_charge_lock_contended").increment();
                TwilioGenericPayResponse response = Adapter::failure_response(Failure::make(
                    error_codes::PAYMENT_PENDING, "A charge for this payment is already in flight"));
                send_response(res, send, encode_response(response));
                return;
            }
        }

        const auto authenticator = Authenticator{};
        auto auth = authenticator.authenticate(*m_context, req, merchant_id, request);
        if (!auth.ok) {
            if (lock) {
                m_context->locks().release(action.key);
            }
            send_api_error(
                res, send,
                encode_api_error(
                    "invalid_request", "API key not provided or invalid API key used", "IR_01"),
                interfaces::io::types::Status::UNAUTHORIZED);
            return;
        }

        TwilioGenericPayResponse response =
            charge<Adapter>(state_ref{}, auth.value, request, payment_id, transaction_reference);

        if (lock) {
            m_context->locks().release(action.key);
        }
        send_response(res, send, encode_response(response));
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/telephone_order"}
            .add_route(
                Route{"/:merchant_id/:provider"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_telephone_order_charge(req, res, std::move(send));
                    })
                    .summary("Hyperswitch telephone_order_charge")
            );
    }

    explicit TelephoneOrderRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct state_ref
    {
    };

    /** @brief core/telephone_order.rs:209-283 — charge (the shared create+confirm flow). */
    template<typename Adapter>
    [[nodiscard]] TwilioGenericPayResponse charge(
        state_ref,
        const AuthenticationData& auth,
        const TwilioGenericPayRequest& request,
        const std::optional<std::string>& route_payment_id,
        const std::string& transaction_reference)
    {
        (void)route_payment_id;
        auto payments_request = TelephoneOrderCore::resolve_payment_id(request)
                                    .and_then([this, &auth, &request](std::optional<std::string> id)
                                    {
                                        return TelephoneOrderCore::build_context(
                                            *m_context, auth, Adapter::parameters(request),
                                            id.value_or(""));
                                    })
                                    .and_then([&request](const Context& context)
                                    {
                                        return Adapter::build_payments_request(request, context);
                                    })
                                    .and_then([this](TelephoneOrderPaymentsRequest value)
                                    {
                                        return TelephoneOrderCore::finalize_payments_request(
                                            std::move(value), m_context->ids());
                                    });

        if (!payments_request.ok) {
            return Adapter::report_failure(
                *m_context, transaction_reference, std::move(payments_request.failure),
                "telephone_order_charge");
        }

        const TelephoneOrderPaymentsRequest& mapped = payments_request.value;
        const std::string& payment_id = mapped.payment_id;

        // payments_core substitute: a retry collides with the existing payment and replays its
        // own outcome instead of charging the card again (core/telephone_order.rs:263-274).
        const auto existing = m_context->repository().find_payment_by_string(payment_id);
        if (existing) {
            m_context->metrics().counter("telephone_order_charge_duplicate_replay").increment();
            m_context->metrics().record_flow("TelephoneOrderChargeDuplicateReplay");
            return answer<Adapter>(transaction_reference, retrieve_payment(payment_id, *existing));
        }

        app::connector::ConnectorRequest connector_request =
            app::connector::ConnectorRequestBuilder{}
                .operation("authorize")
                .merchant_id(mapped.merchant_id)
                .amount(mapped.amount)
                .currency(mapped.currency)
                .payment_method_type("card")
                .resource_id(payment_id)
                .raw_payload(encode_connector_payload(mapped))();

        auto connector = m_context->connector_router().select_for(connector_request);
        if (!connector) {
            const auto router_error = m_context->connector_router().last_error();
            const auto kind = router_error ? router_error->get_kind()
                                           : core::error::AppErrorKind::CONNECTOR_ERROR;
            const std::string message = router_error ? router_error->get_message()
                                                      : "no connector available";
            m_context->metrics().counter("telephone_order_charge_processing_error").increment();
            return Adapter::report_failure(
                *m_context, transaction_reference,
                TelephoneOrderCore::classify_core_error(kind, message), "telephone_order_charge");
        }

        m_context->metrics().counter("telephone_order_charge_connector_authorize").increment();
        const auto connector_response = connector->authorize(connector_request);
        PaymentView payment{};
        payment.payment_id = payment_id;
        payment.status = TelephoneOrderCore::canonical_status(connector_response.get_status());
        payment.amount_capturable =
            payment.status == "requires_capture" ? mapped.amount : std::int64_t{0};
        if (payment.status == "succeeded" || payment.status == "partially_captured") {
            payment.amount_received = mapped.amount;
        }
        payment.connector = std::string{connector->name()};
        if (!connector_response.get_reference_id().empty()) {
            payment.connector_transaction_id = connector_response.get_reference_id();
        }
        payment.merchant_order_reference_id = mapped.merchant_order_reference_id;
        decode_connector_outcome(connector_response.get_raw_json(), payment);

        persist(payment, mapped);
        return answer<Adapter>(transaction_reference, payment);
    }

    /** @brief core/telephone_order.rs:285-309 — answer with Paid/Declined/Indeterminate. */
    template<typename Adapter>
    [[nodiscard]] TwilioGenericPayResponse answer(
        const std::string& transaction_reference, const PaymentView& payment)
    {
        const Outcome outcome = TelephoneOrderCore::payment_outcome(payment);
        switch (outcome.kind) {
        case OutcomeKind::Paid:
            m_context->metrics().counter("telephone_order_charge_paid").increment();
            return Adapter::success_response(payment, TelephoneOrderCore::outcome_parameters(payment));
        case OutcomeKind::Declined:
            m_context->metrics().counter("telephone_order_charge_declined").increment();
            return Adapter::report_failure(
                *m_context, transaction_reference, outcome.failure, "telephone_order_charge");
        case OutcomeKind::Indeterminate:
        default:
            m_context->metrics().counter("telephone_order_charge_pending").increment();
            m_context->metrics().record_flow("TelephoneOrderChargeIndeterminate");
            return Adapter::report_failure(
                *m_context, transaction_reference, outcome.failure, "telephone_order_charge");
        }
    }

    /**
     * @brief core/telephone_order.rs:446-500 — retrieve_payment via PSync substitute.
     * The lock guarantees the original charge finished, so the stored payment is read back
     * verbatim (force_sync=false, CallConnectorAction::Avoid); no second connector call.
     */
    [[nodiscard]] PaymentView retrieve_payment(
        const std::string& payment_id,
        const app::store::payments::PaymentIntentRecord& record)
    {
        PaymentView payment{};
        payment.payment_id = payment_id;
        payment.status = TelephoneOrderCore::canonical_status(record.get_status());
        payment.amount_capturable = record.get_amount();
        payment.merchant_order_reference_id = record.get_merchant_reference_id();

        const std::string attempt_id = payment_id + "_1";
        if (const auto attempt = m_context->repository().find_payment_attempt_by_string(attempt_id)) {
            payment.connector = attempt->get_connector();
            if (!attempt->get_error_code().empty()) {
                payment.unified_code = attempt->get_error_code();
            }
            if (!attempt->get_error_message().empty()) {
                payment.unified_message = attempt->get_error_message();
            }
        }
        return payment;
    }

    /** @brief Persist the intent + attempt (payments_core create+confirm substitute). */
    void persist(const PaymentView& payment, const TelephoneOrderPaymentsRequest& mapped)
    {
        app::store::payments::PaymentIntentRecord record{};
        record.set_payment_id(payment.payment_id);
        record.set_merchant_id(mapped.merchant_id);
        record.set_merchant_reference_id(mapped.merchant_order_reference_id.value_or(""));
        record.set_customer_id(mapped.customer_id.value_or(""));
        record.set_status(payment.status);
        record.set_amount(mapped.amount);
        record.set_currency(mapped.currency);
        record.set_description(mapped.description.value_or(""));
        record.set_metadata(mapped.metadata.value_or(""));
        m_context->repository().save_payment(std::move(record));

        app::store::payment_attempts::PaymentAttemptRecord attempt{};
        attempt.set_attempt_id(payment.payment_id + "_1");
        attempt.set_payment_id(payment.payment_id);
        attempt.set_merchant_id(mapped.merchant_id);
        attempt.set_connector(payment.connector.value_or(""));
        attempt.set_status(payment.status);
        attempt.set_amount(mapped.amount);
        attempt.set_currency(mapped.currency);
        attempt.set_error_code(payment.unified_code.value_or(""));
        attempt.set_error_message(payment.unified_message.value_or(""));
        m_context->repository().save_payment_attempt(std::move(attempt));
    }

    /** @brief Carry the mapped PaymentsRequest flags on the connector raw_payload. */
    [[nodiscard]] static std::string encode_connector_payload(
        const TelephoneOrderPaymentsRequest& mapped)
    {
        std::string out{"{"};
        out += R"("amount":)" + std::to_string(mapped.amount);
        out += R"(,"currency":)" + json_string(mapped.currency);
        out += R"(,"payment_method":"card")";
        out += R"(,"payment_id":)" + json_string(mapped.payment_id);
        out += R"(,"merchant_id":)" + json_string(mapped.merchant_id);
        out += R"(,"confirm":true)";
        out += R"(,"capture_method":"automatic")";
        out += R"(,"authentication_type":"no_three_ds")";
        out += R"(,"payment_channel":"telephone_order")";
        out += R"(,"merchant_order_reference_id":)" +
               json_nullable(mapped.merchant_order_reference_id);
        out += R"(,"description":)" + json_nullable(mapped.description);
        out += R"(,"card_last4":)" + json_nullable(mapped.card_last4);
        out += R"(,"billing_postal_code":)" + json_nullable(mapped.billing_zip);
        out += R"(,"metadata":)" + json_nullable(mapped.metadata);
        out += R"(,"profile_id":)" + json_nullable(mapped.profile_id);
        out += R"(,"customer_id":)" + json_nullable(mapped.customer_id);
        out.push_back('}');
        return out;
    }

    /** @brief Decode connector decline codes from its raw JSON for decline_failure. */
    static void decode_connector_outcome(std::string_view raw, PaymentView& payment)
    {
        if (raw.empty()) {
            return;
        }
        const auto read = [raw](std::string_view key) -> std::optional<std::string>
        {
            const std::string pattern{"\"" + std::string{key} + "\""};
            const std::size_t key_pos = raw.find(pattern);
            if (key_pos == std::string_view::npos) {
                return std::nullopt;
            }
            std::size_t cursor = key_pos + pattern.size();
            while (cursor < raw.size() && std::isspace(static_cast<unsigned char>(raw[cursor]))) {
                ++cursor;
            }
            if (cursor >= raw.size() || raw[cursor] != ':') {
                return std::nullopt;
            }
            ++cursor;
            while (cursor < raw.size() && std::isspace(static_cast<unsigned char>(raw[cursor]))) {
                ++cursor;
            }
            if (cursor >= raw.size() || raw[cursor] != '"') {
                return std::nullopt;
            }
            ++cursor;
            const std::size_t end = raw.find('"', cursor);
            if (end == std::string_view::npos) {
                return std::nullopt;
            }
            return std::string{raw.substr(cursor, end - cursor)};
        };
        if (const auto code = read("unified_code")) {
            payment.unified_code = code;
        } else if (const auto error_code = read("error_code")) {
            payment.unified_code = error_code;
        }
        if (const auto message = read("unified_message")) {
            payment.unified_message = message;
        } else if (const auto error_message = read("error_message")) {
            payment.unified_message = error_message;
        }
    }

    /** @brief services/authentication.rs — local ApiKeyBasicAuthWithMerchantIdFromRoute. */
    class Authenticator
    {
    public:
        [[nodiscard]] Result<AuthenticationData> authenticate(
            app::context::AppContext& context,
            interfaces::io::IRequest& req,
            const std::string& merchant_id,
            const TwilioGenericPayRequest& request) const
        {
            auto key = basic_api_key(req);
            app::auth::AuthContext auth_ctx = app::auth::AuthContextBuilder{}(req);
            if (key) {
                auth_ctx.add_claim(std::string{app::auth::kClaimApiKey}, *key);
            }
            auth_ctx.set_merchant_id(merchant_id);

            const app::auth::RouteAuthRule rule = app::auth::RouteAuthRuleBuilder{}
                                                      .scheme(app::auth::AuthScheme::MERCHANT_ID)
                                                      .permission(app::auth::AppPermission::PAYMENT_WRITE)
                                                      .id_source(std::string{app::auth::kClaimApiKey})
                                                      .build();
            if (context.authorizer().authorize_route(auth_ctx, rule)) {
                return Result<AuthenticationData>::error(Failure::make(
                    error_codes::INVALID_REQUEST, "API key not provided or invalid API key used"));
            }

            AuthenticationData auth{};
            auth.merchant_id = merchant_id;
            // authenticator() hands extract_id(parameters,"profile_id").ok().flatten() to auth;
            // charge() re-parses it and rejects an unusable id there.
            auto profile = TelephoneOrderCore::extract_id(
                TwilioGenericPay::parameters(request), "profile_id");
            if (profile.ok && profile.value) {
                auth.profile_id = std::move(profile.value);
            } else {
                const auto account = context.repository().find_merchant_account_by_string(merchant_id);
                if (account && !account->get_default_profile().empty()) {
                    auth.profile_id = account->get_default_profile();
                }
            }
            return Result<AuthenticationData>::success(std::move(auth));
        }
    };

    void send_api_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string text,
        interfaces::io::types::Status status) const
    {
        res.set_status(status);
        res.set_body(serde::Ser::serialize_raw("application/json", text));
        send();
    }

    void send_response(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string text) const
    {
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize_raw("application/json", text));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::telephone_order

// ---------------------------------------------------------------------------
// serde reflection for the provider wire shapes.
// ---------------------------------------------------------------------------
template<>
struct serde::Serializable<app::handlers::telephone_order::TwilioGenericPayRequest>
{
    static constexpr auto fields()
    {
        using Wire = app::handlers::telephone_order::TwilioGenericPayRequest;
        return std::tuple{
            serde::FieldDesc<"method", &Wire::get_method, &Wire::set_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"transaction_id", &Wire::get_transaction_id, &Wire::set_transaction_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"cardnumber", &Wire::get_cardnumber, &Wire::set_cardnumber, serde::FieldOptions{}>{},
            serde::FieldDesc<"cvv", &Wire::get_cvv, &Wire::set_cvv, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_month", &Wire::get_expiry_month, &Wire::set_expiry_month, serde::FieldOptions{}>{},
            serde::FieldDesc<"expiry_year", &Wire::get_expiry_year, &Wire::set_expiry_year, serde::FieldOptions{}>{},
            serde::FieldDesc<"postal_code", &Wire::get_postal_code, &Wire::set_postal_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"bankaccountnumber", &Wire::get_bankaccountnumber, &Wire::set_bankaccountnumber, serde::FieldOptions{}>{},
            serde::FieldDesc<"routingnumber", &Wire::get_routingnumber, &Wire::set_routingnumber, serde::FieldOptions{}>{},
            serde::FieldDesc<"amount", &Wire::get_amount, &Wire::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency_code", &Wire::get_currency_code, &Wire::set_currency_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &Wire::get_description, &Wire::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"parameters", &Wire::get_parameters, &Wire::set_parameters, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "twilio_generic_pay_request"; }
};

template<>
struct serde::Serializable<app::handlers::telephone_order::TwilioGenericPayResponse>
{
    static constexpr auto fields()
    {
        using Wire = app::handlers::telephone_order::TwilioGenericPayResponse;
        return std::tuple{
            serde::FieldDesc<"charge_id", &Wire::get_charge_id, &Wire::set_charge_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &Wire::get_error_code, &Wire::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &Wire::get_error_message, &Wire::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"parameters", &Wire::get_parameters, &Wire::set_parameters, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "twilio_generic_pay_response"; }
};
