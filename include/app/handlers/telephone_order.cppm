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

// Local typed wire shapes for the upstream api_models telephone_order types. The
// app_dto partition only carries a catalog placeholder, so the provider contract
// is mirrored here. Names and nesting follow api_models/telephone_order/twilio.rs.
class TwilioPayRequestWire
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
    void set_parameters(std::unordered_map<std::string, std::string> value) { parameters = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_parameters() const noexcept { return parameters; }

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
    std::unordered_map<std::string, std::string> parameters{};
};

class TwilioPayResponseWire
{
public:
    void set_charge_id(std::optional<std::string> value) { charge_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_charge_id() const noexcept { return charge_id; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }
    void set_parameters(std::unordered_map<std::string, std::string> value) { parameters = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_parameters() const noexcept { return parameters; }

    std::optional<std::string> charge_id{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::unordered_map<std::string, std::string> parameters{};
};

class TelephoneOrderParametersWire
{
public:
    void set_parameters(std::unordered_map<std::string, std::string> value) { parameters = std::move(value); }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_parameters() const noexcept { return parameters; }
    std::unordered_map<std::string, std::string> parameters{};
};

// Normalized PaymentsRequest substitute: the connector request has no dedicated
// fields for the flags, so they are carried in raw_payload as a typed shape.
class TelephoneOrderConnectorPayloadWire
{
public:
    void set_amount(std::int64_t value) { amount = value; }
    [[nodiscard]] const std::int64_t& get_amount() const noexcept { return amount; }
    void set_currency(std::string value) { currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return currency; }
    void set_payment_method(std::string value) { payment_method = std::move(value); }
    [[nodiscard]] const std::string& get_payment_method() const noexcept { return payment_method; }
    void set_payment_id(std::string value) { payment_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_id() const noexcept { return payment_id; }
    void set_merchant_id(std::string value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return merchant_id; }
    void set_confirm(bool value) { confirm = value; }
    [[nodiscard]] const bool& get_confirm() const noexcept { return confirm; }
    void set_capture_method(std::string value) { capture_method = std::move(value); }
    [[nodiscard]] const std::string& get_capture_method() const noexcept { return capture_method; }
    void set_authentication_type(std::string value) { authentication_type = std::move(value); }
    [[nodiscard]] const std::string& get_authentication_type() const noexcept { return authentication_type; }
    void set_payment_channel(std::string value) { payment_channel = std::move(value); }
    [[nodiscard]] const std::string& get_payment_channel() const noexcept { return payment_channel; }
    void set_merchant_order_reference_id(std::string value) { merchant_order_reference_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_order_reference_id() const noexcept { return merchant_order_reference_id; }
    void set_description(std::optional<std::string> value) { description = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_description() const noexcept { return description; }
    void set_card_last4(std::optional<std::string> value) { card_last4 = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_card_last4() const noexcept { return card_last4; }
    void set_billing_postal_code(std::optional<std::string> value) { billing_postal_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_billing_postal_code() const noexcept { return billing_postal_code; }
    void set_metadata(std::optional<std::string> value) { metadata = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_metadata() const noexcept { return metadata; }
    void set_profile_id(std::optional<std::string> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_profile_id() const noexcept { return profile_id; }
    void set_customer_id(std::optional<std::string> value) { customer_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_customer_id() const noexcept { return customer_id; }

    std::int64_t amount{};
    std::string currency{};
    std::string payment_method{};
    std::string payment_id{};
    std::string merchant_id{};
    bool confirm{};
    std::string capture_method{};
    std::string authentication_type{};
    std::string payment_channel{};
    std::string merchant_order_reference_id{};
    std::optional<std::string> description{};
    std::optional<std::string> card_last4{};
    std::optional<std::string> billing_postal_code{};
    std::optional<std::string> metadata{};
    std::optional<std::string> profile_id{};
    std::optional<std::string> customer_id{};
};

// The connector has no unified_code field on its response, so the deterministic
// local connector's raw_json is decoded into this shape for decline codes.
class ConnectorOutcomeWire
{
public:
    void set_unified_code(std::optional<std::string> value) { unified_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unified_code() const noexcept { return unified_code; }
    void set_error_code(std::optional<std::string> value) { error_code = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept { return error_code; }
    void set_unified_message(std::optional<std::string> value) { unified_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_unified_message() const noexcept { return unified_message; }
    void set_error_message(std::optional<std::string> value) { error_message = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept { return error_message; }

    std::optional<std::string> unified_code{};
    std::optional<std::string> error_code{};
    std::optional<std::string> unified_message{};
    std::optional<std::string> error_message{};
};

enum class ChargeOutcome : std::uint8_t
{
    Paid,
    Declined,
    Indeterminate,
};

/** @brief Validates the Hyperswitch ids the IVR passes in `parameters`. */
class TelephoneOrderIdValidator
{
public:
    [[nodiscard]] bool is_payment_id(std::string_view value) const noexcept
    {
        return shaped(value, "pay_", 20);
    }

    [[nodiscard]] bool is_profile_id(std::string_view value) const noexcept
    {
        return shaped(value, "pro_", 8);
    }

    [[nodiscard]] bool is_customer_id(std::string_view value) const noexcept
    {
        return shaped(value, "cus_", 8);
    }

private:
    [[nodiscard]] bool shaped(std::string_view value, std::string_view prefix, std::size_t minimum) const noexcept
    {
        if (value.size() < prefix.size() + minimum || !value.starts_with(prefix)) {
            return false;
        }
        for (const char character: value.substr(prefix.size())) {
            const auto byte = static_cast<unsigned char>(character);
            if (!std::isalnum(byte) && character != '_' && character != '-') {
                return false;
            }
        }
        return true;
    }
};

/** @brief Deterministic StringMajorUnitForConnector substitute (major-unit string -> minor units). */
class MajorUnitAmountParser
{
public:
    [[nodiscard]] std::string normalize_currency(std::string_view value) const
    {
        std::string result{};
        for (const char character: value) {
            result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(character))));
        }
        return result;
    }

    [[nodiscard]] bool is_currency(std::string_view value) const noexcept
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

    [[nodiscard]] std::optional<std::int64_t> parse(std::string_view text, std::string_view currency) const
    {
        const int digits = minor_digits(currency);
        std::int64_t major{};
        std::int64_t fraction{};
        int fraction_length{};
        bool seen_digit{};
        bool seen_dot{};

        for (const char character: text) {
            if (character == '.' && !seen_dot) {
                seen_dot = true;
                continue;
            }
            if (!std::isdigit(static_cast<unsigned char>(character))) {
                return std::nullopt;
            }
            const std::int64_t digit = character - '0';
            seen_digit = true;
            if (!seen_dot) {
                major = major * 10 + digit;
            } else if (fraction_length < 6) {
                fraction = fraction * 10 + digit;
                ++fraction_length;
            } else {
                return std::nullopt;
            }
        }
        if (!seen_digit || major < 0) {
            return std::nullopt;
        }

        const std::int64_t factor = power_of_ten(digits);
        std::int64_t scaled{};
        if (fraction_length <= digits) {
            scaled = fraction * power_of_ten(digits - fraction_length);
        } else {
            const std::int64_t divisor = power_of_ten(fraction_length - digits);
            scaled = fraction / divisor;
            if ((fraction % divisor) * 2 >= divisor) {
                ++scaled;
            }
        }
        return major * factor + scaled;
    }

private:
    [[nodiscard]] int minor_digits(std::string_view currency) const noexcept
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

    [[nodiscard]] std::int64_t power_of_ten(int exponent) const noexcept
    {
        std::int64_t result{1};
        for (int index = 0; index < exponent; ++index) {
            result *= 10;
        }
        return result;
    }
};

/** @brief Local substitute for classify_core_error / payment_outcome / decline_failure. */
class TelephoneOrderClassifier
{
public:
    [[nodiscard]] ChargeOutcome outcome(std::string_view status) const noexcept
    {
        if (status == "succeeded" || status == "partially_captured" || status == "requires_capture" ||
            status == "partially_authorized_and_requires_capture" || status == "authorized" ||
            status == "captured") {
            return ChargeOutcome::Paid;
        }
        if (status.empty() || status == "pending" || status == "processing" || status == "synced" ||
            status == "partially_captured_and_capturable" || status == "partially_captured_and_processing" ||
            status == "review" || status == "requires_merchant_action" || status == "conflicted" ||
            status == "cancelled_post_capture") {
            return ChargeOutcome::Indeterminate;
        }
        return ChargeOutcome::Declined;
    }

    [[nodiscard]] std::pair<std::string, std::string>
    classify(core::error::AppErrorKind kind, std::string_view message) const
    {
        switch (kind) {
        case core::error::AppErrorKind::INVALID_REQUEST:
        case core::error::AppErrorKind::INVALID_JSON:
        case core::error::AppErrorKind::EMPTY_REQUEST:
        case core::error::AppErrorKind::VALIDATION_FAILED:
        case core::error::AppErrorKind::PRECONDITION_FAILED:
            return {"INVALID_REQUEST", std::string{message}};
        case core::error::AppErrorKind::NOT_FOUND:
            return {"PROFILE_NOT_FOUND", std::string{message}};
        default:
            return {"PROCESSING_ERROR", "Payment could not be processed"};
        }
    }

    [[nodiscard]] std::string decline_code(
        const ConnectorOutcomeWire& wire) const
    {
        if (wire.get_unified_code() && !wire.get_unified_code()->empty()) {
            return *wire.get_unified_code();
        }
        if (wire.get_error_code() && !wire.get_error_code()->empty()) {
            return *wire.get_error_code();
        }
        return "PAYMENT_NOT_COMPLETED";
    }

    [[nodiscard]] std::string decline_message(
        const ConnectorOutcomeWire& wire, std::string_view status) const
    {
        if (wire.get_unified_message() && !wire.get_unified_message()->empty()) {
            return *wire.get_unified_message();
        }
        if (wire.get_error_message() && !wire.get_error_message()->empty()) {
            return *wire.get_error_message();
        }
        return std::format("Payment was not completed: {}", status);
    }
};

class TelephoneOrderRouteHandlers
{
public:
    void handle_telephone_order_charge(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("telephone_order_charge");
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(auth_ctx, rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (count < 2) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id in route"));
            return;
        }
        const auto merchant_segment = segments.at(count - 2);
        const auto provider_segment = segments.at(count - 1);
        if (!merchant_segment || merchant_segment->empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id in route"));
            return;
        }
        const std::string merchant_id{*merchant_segment};
        const std::string provider{provider_segment ? std::string{*provider_segment} : std::string{}};
        if (provider != "twilio") {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "InvalidRequestUrl: unknown telephone order provider: " + provider));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<TwilioPayRequestWire>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "InvalidDataFormat: request body is not a telephone order charge request for this provider"));
            return;
        }

        m_context->metrics().record_flow("TelephoneOrderCharge");
        m_context->metrics().counter("telephone_order_charge_total").increment();

        const TelephoneOrderIdValidator id_validator{};
        const MajorUnitAmountParser amount_parser{};
        const TelephoneOrderClassifier classifier{};
        const auto& input = *parsed;
        const auto& parameters = input.get_parameters();

        TwilioPayResponseWire response{};
        std::unordered_map<std::string, std::string> out_parameters{};

        const auto set_failure = [&response](std::string_view code, std::string message)
        {
            response.set_charge_id(std::nullopt);
            response.set_error_code(std::string{code});
            response.set_error_message(std::move(message));
        };
        const auto finish = [this, &res, &send, &response, &out_parameters]()
        {
            response.set_parameters(out_parameters);
            auto text = serde::Ser::encode_json(response);
            send_json(res, send, text, true);
        };
        const auto fail_and_finish = [&set_failure, &finish](std::string_view code, std::string message)
        {
            set_failure(code, std::move(message));
            finish();
        };

        // parameters.payment_id wins; otherwise the provider's stable transaction id is the
        // idempotency material. The digest is domain-separated and 32 hex chars, matching the
        // pay_ + SHA256[:16] shape without claiming a cryptographic hash.
        std::string payment_id{};
        const auto supplied = parameters.find("payment_id");
        if (supplied != parameters.end()) {
            if (!id_validator.is_payment_id(supplied->second)) {
                m_context->metrics().counter("telephone_order_charge_invalid_request").increment();
                fail_and_finish("INVALID_REQUEST", "parameters.payment_id is not a valid payment_id");
                return;
            }
            payment_id = supplied->second;
        } else {
            if (input.get_transaction_id().empty()) {
                m_context->metrics().counter("telephone_order_charge_invalid_request").increment();
                fail_and_finish("INVALID_REQUEST", "transaction_id is required");
                return;
            }
            const app::crypto::KeyedHasher hasher{"telephone_order.transaction_id"};
            const auto head = hasher.hash_with_domain(
                app::crypto::HashDomain::Idempotency, input.get_transaction_id());
            const auto tail = hasher.hash_with_domain(
                "telephone_order.transaction_id.tail", input.get_transaction_id());
            payment_id = "pay_" + head + tail;
        }

        if (input.get_method() != "charge") {
            m_context->metrics().counter("telephone_order_charge_tokenize_rejected").increment();
            fail_and_finish("TOKENIZE_UNSUPPORTED", "Tokenization is not supported on this endpoint");
            return;
        }
        if (input.get_bankaccountnumber() || input.get_routingnumber()) {
            m_context->metrics().counter("telephone_order_charge_bank_rejected").increment();
            fail_and_finish("BANK_ACCOUNT_UNSUPPORTED", "Bank account payments are not supported on this endpoint");
            return;
        }
        if (!input.get_cardnumber() || input.get_cardnumber()->empty()) {
            fail_and_finish("INVALID_REQUEST", "cardnumber is required for a card charge");
            return;
        }
        if (!input.get_expiry_month() || input.get_expiry_month()->empty()) {
            fail_and_finish("INVALID_REQUEST", "expiry_month is required for a card charge");
            return;
        }
        if (!input.get_expiry_year() || input.get_expiry_year()->empty()) {
            fail_and_finish("INVALID_REQUEST", "expiry_year is required for a card charge");
            return;
        }
        if (!input.get_cvv() || input.get_cvv()->empty()) {
            fail_and_finish("INVALID_REQUEST", "cvv is required for a card charge");
            return;
        }

        std::string currency = input.get_currency_code().value_or("");
        currency = amount_parser.normalize_currency(currency);
        if (!amount_parser.is_currency(currency)) {
            fail_and_finish("INVALID_REQUEST", "currency_code is required and must be a 3-letter currency");
            return;
        }
        if (!input.get_amount()) {
            fail_and_finish("INVALID_REQUEST", "amount is required");
            return;
        }
        const auto minor_amount = amount_parser.parse(*input.get_amount(), currency);
        if (!minor_amount || *minor_amount <= 0) {
            fail_and_finish("INVALID_REQUEST", "amount could not be parsed");
            return;
        }

        std::optional<std::string> profile_id{};
        const auto supplied_profile = parameters.find("profile_id");
        if (supplied_profile != parameters.end()) {
            if (!id_validator.is_profile_id(supplied_profile->second)) {
                fail_and_finish("INVALID_REQUEST", "parameters.profile_id is not a valid profile_id");
                return;
            }
            if (!auth_ctx.profile_id().empty() && auth_ctx.profile_id() != supplied_profile->second) {
                m_context->metrics().counter("telephone_order_charge_profile_not_found").increment();
                fail_and_finish("PROFILE_NOT_FOUND",
                                "profile_id " + supplied_profile->second + " is not available for this merchant");
                return;
            }
            profile_id = supplied_profile->second;
        } else if (!auth_ctx.profile_id().empty()) {
            profile_id = auth_ctx.profile_id();
        }

        std::optional<std::string> customer_id{};
        const auto supplied_customer = parameters.find("customer_id");
        if (supplied_customer != parameters.end()) {
            if (id_validator.is_customer_id(supplied_customer->second)) {
                customer_id = supplied_customer->second;
            } else {
                m_context->metrics().counter("telephone_order_charge_customer_id_dropped").increment();
            }
        }

        TelephoneOrderParametersWire metadata_wire{};
        metadata_wire.set_parameters(parameters);
        const std::string metadata = serde::Ser::encode_json(metadata_wire);

        out_parameters.emplace("payment_id", payment_id);
        out_parameters.emplace("merchant_order_reference_id", input.get_transaction_id());
        out_parameters.emplace("status", "pending");
        out_parameters.emplace("amount_capturable", "0");

        const std::string card_last4 = input.get_cardnumber()->size() > 4
            ? input.get_cardnumber()->substr(input.get_cardnumber()->size() - 4)
            : *input.get_cardnumber();
        TelephoneOrderConnectorPayloadWire payload{};
        payload.set_amount(*minor_amount);
        payload.set_currency(currency);
        payload.set_payment_method("card");
        payload.set_payment_id(payment_id);
        payload.set_merchant_id(merchant_id);
        payload.set_confirm(true);
        payload.set_capture_method("automatic");
        payload.set_authentication_type("no_three_ds");
        payload.set_payment_channel("telephone_order");
        payload.set_merchant_order_reference_id(input.get_transaction_id());
        payload.set_description(input.get_description());
        payload.set_card_last4(card_last4);
        payload.set_billing_postal_code(input.get_postal_code());
        payload.set_metadata(metadata);
        payload.set_profile_id(profile_id);
        payload.set_customer_id(customer_id);

        const auto connector_request = app::connector::ConnectorRequestBuilder{}
                                           .operation("authorize")
                                           .merchant_id(merchant_id)
                                           .amount(*minor_amount)
                                           .currency(currency)
                                           .payment_method_type("card")
                                           .resource_id(payment_id)
                                           .raw_payload(serde::Ser::encode_json(payload))();

        const std::string lock_key = "telephone_order:" + payment_id;
        const auto lock = m_context->locks().acquire(
            lock_key, std::int64_t{30}, "telephone_order_charge");
        if (!lock) {
            m_context->metrics().counter("telephone_order_charge_lock_contended").increment();
            out_parameters["status"] = "pending";
            set_failure("PAYMENT_PENDING", "A charge for this payment is already in flight");
            finish();
            return;
        }

        const auto existing = m_context->repository().find_payment_by_string(payment_id);
        const bool duplicate = existing.has_value();
        if (duplicate) {
            m_context->metrics().counter("telephone_order_charge_duplicate_replay").increment();
            m_context->metrics().record_flow("TelephoneOrderChargeDuplicateReplay");
        }

        std::string charge_status{};
        std::string charge_reference{};
        std::string charge_message{};
        std::string charge_raw_json{};
        std::string connector_name{};
        if (auto connector = m_context->connector_router().select_for(connector_request)) {
            connector_name = std::string{connector->name()};
            // A retry reads the original payment back (PSync substitute) instead of
            // authorizing the card a second time.
            const auto connector_response = duplicate
                ? connector->sync(connector_request)
                : connector->authorize(connector_request);
            charge_status = connector_response.get_status();
            charge_reference = connector_response.get_reference_id();
            charge_message = connector_response.get_message();
            charge_raw_json = connector_response.get_raw_json();
            if (!duplicate) {
                m_context->metrics().counter("telephone_order_charge_connector_authorize").increment();
            }
        } else {
            const auto router_error = m_context->connector_router().last_error();
            const auto classified = classifier.classify(
                router_error ? router_error->get_kind() : core::error::AppErrorKind::CONNECTOR_ERROR,
                router_error ? router_error->get_message() : "no connector available");
            m_context->metrics().counter("telephone_order_charge_processing_error").increment();
            m_context->locks().release(lock_key);
            set_failure(classified.first, classified.second);
            finish();
            return;
        }

        std::string final_status{charge_status};
        if (duplicate && existing) {
            // The replayed charge's own status wins over a generic sync status.
            if (!existing->get_status().empty()) {
                final_status = existing->get_status();
            }
            if (!charge_reference.empty()) {
                out_parameters["connector_transaction_id"] = charge_reference;
            }
        }

        ConnectorOutcomeWire connector_outcome{};
        if (!charge_raw_json.empty()) {
            auto decoded = serde::Ser::deserialize<ConnectorOutcomeWire>("application/json", charge_raw_json);
            if (decoded) {
                connector_outcome = *decoded;
            }
        }

        const ChargeOutcome charge_outcome = classifier.outcome(final_status);
        out_parameters["status"] = final_status;
        if (!connector_name.empty()) {
            out_parameters["connector"] = connector_name;
        }
        if (!duplicate && !charge_reference.empty()) {
            out_parameters["connector_transaction_id"] = charge_reference;
        }

        app::store::payments::PaymentIntentRecord record{};
        record.set_payment_id(payment_id);
        record.set_merchant_id(merchant_id);
        record.set_merchant_reference_id(input.get_transaction_id());
        record.set_customer_id(customer_id.value_or(""));
        record.set_status(final_status);
        record.set_amount(*minor_amount);
        record.set_currency(currency);
        record.set_description(input.get_description().value_or(""));
        record.set_metadata(metadata);
        if (existing) {
            record.set_id(existing->get_id());
            m_context->repository().update_payment(record);
        } else {
            m_context->repository().save_payment(std::move(record));
        }

        m_context->locks().release(lock_key);

        switch (charge_outcome) {
        case ChargeOutcome::Paid:
            out_parameters["amount_received"] = std::to_string(*minor_amount);
            response.set_charge_id(payment_id);
            response.set_error_code(std::nullopt);
            response.set_error_message(std::nullopt);
            m_context->metrics().counter("telephone_order_charge_paid").increment();
            break;
        case ChargeOutcome::Declined: {
            const auto code = classifier.decline_code(connector_outcome);
            const auto message = classifier.decline_message(connector_outcome, final_status);
            if (!code.empty()) {
                out_parameters["unified_code"] = code;
            }
            set_failure(code, message);
            m_context->metrics().counter("telephone_order_charge_declined").increment();
            break;
        }
        case ChargeOutcome::Indeterminate:
            set_failure("PAYMENT_PENDING",
                        "Payment outcome is not final and funds may be held: " + final_status);
            m_context->metrics().counter("telephone_order_charge_pending").increment();
            m_context->metrics().record_flow("TelephoneOrderChargeIndeterminate");
            break;
        }
        (void)charge_message;
        finish();
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
    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::telephone_order

template<>
struct serde::Serializable<app::handlers::telephone_order::TwilioPayRequestWire>
{
    static constexpr auto fields()
    {
        using Wire = app::handlers::telephone_order::TwilioPayRequestWire;
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
    static constexpr std::string_view table_name() { return "twilio_pay_request_wire"; }
};

template<>
struct serde::Serializable<app::handlers::telephone_order::TwilioPayResponseWire>
{
    static constexpr auto fields()
    {
        using Wire = app::handlers::telephone_order::TwilioPayResponseWire;
        return std::tuple{
            serde::FieldDesc<"charge_id", &Wire::get_charge_id, &Wire::set_charge_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &Wire::get_error_code, &Wire::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &Wire::get_error_message, &Wire::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"parameters", &Wire::get_parameters, &Wire::set_parameters, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "twilio_pay_response_wire"; }
};

template<>
struct serde::Serializable<app::handlers::telephone_order::TelephoneOrderParametersWire>
{
    static constexpr auto fields()
    {
        using Wire = app::handlers::telephone_order::TelephoneOrderParametersWire;
        return std::tuple{
            serde::FieldDesc<"parameters", &Wire::get_parameters, &Wire::set_parameters, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "telephone_order_parameters_wire"; }
};

template<>
struct serde::Serializable<app::handlers::telephone_order::TelephoneOrderConnectorPayloadWire>
{
    static constexpr auto fields()
    {
        using Wire = app::handlers::telephone_order::TelephoneOrderConnectorPayloadWire;
        return std::tuple{
            serde::FieldDesc<"amount", &Wire::get_amount, &Wire::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &Wire::get_currency, &Wire::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_method", &Wire::get_payment_method, &Wire::set_payment_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &Wire::get_payment_id, &Wire::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &Wire::get_merchant_id, &Wire::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"confirm", &Wire::get_confirm, &Wire::set_confirm, serde::FieldOptions{}>{},
            serde::FieldDesc<"capture_method", &Wire::get_capture_method, &Wire::set_capture_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"authentication_type", &Wire::get_authentication_type, &Wire::set_authentication_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_channel", &Wire::get_payment_channel, &Wire::set_payment_channel, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_order_reference_id", &Wire::get_merchant_order_reference_id, &Wire::set_merchant_order_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &Wire::get_description, &Wire::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"card_last4", &Wire::get_card_last4, &Wire::set_card_last4, serde::FieldOptions{}>{},
            serde::FieldDesc<"billing_postal_code", &Wire::get_billing_postal_code, &Wire::set_billing_postal_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"metadata", &Wire::get_metadata, &Wire::set_metadata, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &Wire::get_profile_id, &Wire::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"customer_id", &Wire::get_customer_id, &Wire::set_customer_id, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "telephone_order_connector_payload_wire"; }
};

template<>
struct serde::Serializable<app::handlers::telephone_order::ConnectorOutcomeWire>
{
    static constexpr auto fields()
    {
        using Wire = app::handlers::telephone_order::ConnectorOutcomeWire;
        return std::tuple{
            serde::FieldDesc<"unified_code", &Wire::get_unified_code, &Wire::set_unified_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_code", &Wire::get_error_code, &Wire::set_error_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"unified_message", &Wire::get_unified_message, &Wire::set_unified_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &Wire::get_error_message, &Wire::set_error_message, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "connector_outcome_wire"; }
};
