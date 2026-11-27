module;
export module app_connector:model_applepay_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

enum class ApplePayAddressParameters
{
    PostalAddress,
    Phone,
    Email,
};

enum class ApplePayCombinedMetadata
{
    Simplified,
    Manual,
};

class ApplePayPaymentDataValue : public serde::Serializer<ApplePayPaymentDataValue>
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class ApplePayPaymentTiming
{
    Immediate,
    Recurring,
};

class ApplePayRedirectData : public serde::Serializer<ApplePayRedirectData>
{
public:
};

enum class ApplePaySessionResponse
{
    ThirdPartySdk,
    NoThirdPartySdk,
    NoSessionResponse,
};

class ApplepayErrorResponse : public serde::Serializer<ApplepayErrorResponse>
{
public:
    void set_status_code(std::string value)
    {
        status_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_status_code() const noexcept
    {
        return status_code;
    }

    void set_status_message(std::string value)
    {
        status_message = std::move(value);
    }

    [[nodiscard]] const std::string& get_status_message() const noexcept
    {
        return status_message;
    }

    std::string status_code{};
    std::string status_message{};
};

class ApplepaySessionRequest : public serde::Serializer<ApplepaySessionRequest>
{
public:
    void set_merchant_identifier(std::string value)
    {
        merchant_identifier = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_identifier() const noexcept
    {
        return merchant_identifier;
    }

    void set_display_name(std::string value)
    {
        display_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_display_name() const noexcept
    {
        return display_name;
    }

    void set_initiative(std::string value)
    {
        initiative = std::move(value);
    }

    [[nodiscard]] const std::string& get_initiative() const noexcept
    {
        return initiative;
    }

    void set_initiative_context(std::string value)
    {
        initiative_context = std::move(value);
    }

    [[nodiscard]] const std::string& get_initiative_context() const noexcept
    {
        return initiative_context;
    }

    std::string merchant_identifier{};
    std::string display_name{};
    std::string initiative{};
    std::string initiative_context{};
};

enum class ApplepaySessionTokenMetadata
{
    ApplePayCombined,
    ApplePay,
};

class ApplePayBillingContactFields : public serde::Serializer<ApplePayBillingContactFields>
{
public:
    void set_value(std::vector<ApplePayAddressParameters> value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::vector<ApplePayAddressParameters>& get_value() const noexcept
    {
        return value;
    }

    std::vector<ApplePayAddressParameters> value{};
};

class ApplePayCombinedWrapper : public serde::Serializer<ApplePayCombinedWrapper>
{
public:
    void set_data(std::optional<ApplePayCombinedMetadata> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::optional<ApplePayCombinedMetadata>& get_data() const noexcept
    {
        return data;
    }

    void set_support_predecrypted_token(std::optional<bool> value)
    {
        support_predecrypted_token = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_support_predecrypted_token() const noexcept
    {
        return support_predecrypted_token;
    }

    std::optional<ApplePayCombinedMetadata> data{};
    std::optional<bool> support_predecrypted_token{};
};

class ApplePayRegularBillingRequest : public serde::Serializer<ApplePayRegularBillingRequest>
{
public:
    void set_amount(StringMajorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_label(std::string value)
    {
        label = std::move(value);
    }

    [[nodiscard]] const std::string& get_label() const noexcept
    {
        return label;
    }

    void set_payment_timing(ApplePayPaymentTiming value)
    {
        payment_timing = std::move(value);
    }

    [[nodiscard]] const ApplePayPaymentTiming& get_payment_timing() const noexcept
    {
        return payment_timing;
    }

    void set_recurring_payment_start_date(std::optional<PrimitiveDateTimeValue> value)
    {
        recurring_payment_start_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_recurring_payment_start_date() const noexcept
    {
        return recurring_payment_start_date;
    }

    void set_recurring_payment_end_date(std::optional<PrimitiveDateTimeValue> value)
    {
        recurring_payment_end_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>&
    get_recurring_payment_end_date() const noexcept
    {
        return recurring_payment_end_date;
    }

    void set_recurring_payment_interval_unit(std::optional<RecurringPaymentIntervalUnit> value)
    {
        recurring_payment_interval_unit = std::move(value);
    }

    [[nodiscard]] const std::optional<RecurringPaymentIntervalUnit>&
    get_recurring_payment_interval_unit() const noexcept
    {
        return recurring_payment_interval_unit;
    }

    void set_recurring_payment_interval_count(std::optional<std::int32_t> value)
    {
        recurring_payment_interval_count = std::move(value);
    }

    [[nodiscard]] const std::optional<std::int32_t>&
    get_recurring_payment_interval_count() const noexcept
    {
        return recurring_payment_interval_count;
    }

    StringMajorUnitValue amount{};
    std::string label{};
    ApplePayPaymentTiming payment_timing{};
    std::optional<PrimitiveDateTimeValue> recurring_payment_start_date{};
    std::optional<PrimitiveDateTimeValue> recurring_payment_end_date{};
    std::optional<RecurringPaymentIntervalUnit> recurring_payment_interval_unit{};
    std::optional<std::int32_t> recurring_payment_interval_count{};
};

class ApplePayShippingContactFields : public serde::Serializer<ApplePayShippingContactFields>
{
public:
    void set_value(std::vector<ApplePayAddressParameters> value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::vector<ApplePayAddressParameters>& get_value() const noexcept
    {
        return value;
    }

    std::vector<ApplePayAddressParameters> value{};
};

class ApplePayThirdPartySdkData : public serde::Serializer<ApplePayThirdPartySdkData>
{
public:
    void set_token(std::optional<SecretValue> value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_token() const noexcept
    {
        return token;
    }

    std::optional<SecretValue> token{};
};

class ApplepayPaymentMethod : public serde::Serializer<ApplepayPaymentMethod>
{
public:
    void set_display_name(std::string value)
    {
        display_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_display_name() const noexcept
    {
        return display_name;
    }

    void set_network(std::string value)
    {
        network = std::move(value);
    }

    [[nodiscard]] const std::string& get_network() const noexcept
    {
        return network;
    }

    void set_pm_type(std::string value)
    {
        pm_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_pm_type() const noexcept
    {
        return pm_type;
    }

    void set_card_exp_month(std::optional<SecretValue> value)
    {
        card_exp_month = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_exp_month() const noexcept
    {
        return card_exp_month;
    }

    void set_card_exp_year(std::optional<SecretValue> value)
    {
        card_exp_year = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_exp_year() const noexcept
    {
        return card_exp_year;
    }

    void set_device_pan_bin(std::optional<std::string> value)
    {
        device_pan_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_device_pan_bin() const noexcept
    {
        return device_pan_bin;
    }

    void set_card_bin(std::optional<std::string> value)
    {
        card_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_bin() const noexcept
    {
        return card_bin;
    }

    void set_card_type(std::optional<CardTypeValue> value)
    {
        card_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CardTypeValue>& get_card_type() const noexcept
    {
        return card_type;
    }

    void set_auth_code(std::optional<std::string> value)
    {
        auth_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_auth_code() const noexcept
    {
        return auth_code;
    }

    void set_card_subtype(std::optional<std::string> value)
    {
        card_subtype = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_card_subtype() const noexcept
    {
        return card_subtype;
    }

    void set_card_segment_type(std::optional<CardSegmentTypeValue> value)
    {
        card_segment_type = std::move(value);
    }

    [[nodiscard]] const std::optional<CardSegmentTypeValue>& get_card_segment_type() const noexcept
    {
        return card_segment_type;
    }

    void set_funding_source(std::optional<FundingSourceValue> value)
    {
        funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<FundingSourceValue>& get_funding_source() const noexcept
    {
        return funding_source;
    }

    void set_issuer_name(std::optional<std::string> value)
    {
        issuer_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_issuer_name() const noexcept
    {
        return issuer_name;
    }

    void set_issuer_country(std::optional<CountryAlpha2Value> value)
    {
        issuer_country = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_issuer_country() const noexcept
    {
        return issuer_country;
    }

    void set_payment_checks(std::optional<serde::Value> value)
    {
        payment_checks = std::move(value);
    }

    [[nodiscard]] const std::optional<serde::Value>& get_payment_checks() const noexcept
    {
        return payment_checks;
    }

    std::string display_name{};
    std::string network{};
    std::string pm_type{};
    std::optional<SecretValue> card_exp_month{};
    std::optional<SecretValue> card_exp_year{};
    std::optional<std::string> device_pan_bin{};
    std::optional<std::string> card_bin{};
    std::optional<CardTypeValue> card_type{};
    std::optional<std::string> auth_code{};
    std::optional<std::string> card_subtype{};
    std::optional<CardSegmentTypeValue> card_segment_type{};
    std::optional<FundingSourceValue> funding_source{};
    std::optional<std::string> issuer_name{};
    std::optional<CountryAlpha2Value> issuer_country{};
    std::optional<serde::Value> payment_checks{};
};

class ApplePayMetadata : public serde::Serializer<ApplePayMetadata>
{
public:
    void set_payment_request_data(PaymentRequestMetadata value)
    {
        payment_request_data = std::move(value);
    }

    [[nodiscard]] const PaymentRequestMetadata& get_payment_request_data() const noexcept
    {
        return payment_request_data;
    }

    void set_session_token_data(SessionTokenInfo value)
    {
        session_token_data = std::move(value);
    }

    [[nodiscard]] const SessionTokenInfo& get_session_token_data() const noexcept
    {
        return session_token_data;
    }

    PaymentRequestMetadata payment_request_data{};
    SessionTokenInfo session_token_data{};
};

class ApplePayRecurringPaymentRequest : public serde::Serializer<ApplePayRecurringPaymentRequest>
{
public:
    void set_payment_description(std::string value)
    {
        payment_description = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_description() const noexcept
    {
        return payment_description;
    }

    void set_regular_billing(ApplePayRegularBillingRequest value)
    {
        regular_billing = std::move(value);
    }

    [[nodiscard]] const ApplePayRegularBillingRequest& get_regular_billing() const noexcept
    {
        return regular_billing;
    }

    void set_billing_agreement(std::optional<std::string> value)
    {
        billing_agreement = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_billing_agreement() const noexcept
    {
        return billing_agreement;
    }

    void set_management_u_r_l(UrlValue value)
    {
        management_u_r_l = std::move(value);
    }

    [[nodiscard]] const UrlValue& get_management_u_r_l() const noexcept
    {
        return management_u_r_l;
    }

    std::string payment_description{};
    ApplePayRegularBillingRequest regular_billing{};
    std::optional<std::string> billing_agreement{};
    UrlValue management_u_r_l{};
};

class ApplePayWalletData : public serde::Serializer<ApplePayWalletData>
{
public:
    void set_payment_data(ApplePayPaymentDataValue value)
    {
        payment_data = std::move(value);
    }

    [[nodiscard]] const ApplePayPaymentDataValue& get_payment_data() const noexcept
    {
        return payment_data;
    }

    void set_payment_method(ApplepayPaymentMethod value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const ApplepayPaymentMethod& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_transaction_identifier(std::string value)
    {
        transaction_identifier = std::move(value);
    }

    [[nodiscard]] const std::string& get_transaction_identifier() const noexcept
    {
        return transaction_identifier;
    }

    ApplePayPaymentDataValue payment_data{};
    ApplepayPaymentMethod payment_method{};
    std::string transaction_identifier{};
};

class ApplepayCombinedSessionTokenData : public serde::Serializer<ApplepayCombinedSessionTokenData>
{
public:
    void set_apple_pay_combined(ApplePayCombinedWrapper value)
    {
        apple_pay_combined = std::move(value);
    }

    [[nodiscard]] const ApplePayCombinedWrapper& get_apple_pay_combined() const noexcept
    {
        return apple_pay_combined;
    }

    ApplePayCombinedWrapper apple_pay_combined{};
};

class ApplePayPaymentRequest : public serde::Serializer<ApplePayPaymentRequest>
{
public:
    void set_country_code(CountryAlpha2Value value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_country_code() const noexcept
    {
        return country_code;
    }

    void set_currency_code(CurrencyValue value)
    {
        currency_code = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency_code() const noexcept
    {
        return currency_code;
    }

    void set_total(AmountInfo value)
    {
        total = std::move(value);
    }

    [[nodiscard]] const AmountInfo& get_total() const noexcept
    {
        return total;
    }

    void set_merchant_capabilities(std::optional<std::vector<std::string>> value)
    {
        merchant_capabilities = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_merchant_capabilities() const noexcept
    {
        return merchant_capabilities;
    }

    void set_supported_networks(std::optional<std::vector<std::string>> value)
    {
        supported_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_supported_networks() const noexcept
    {
        return supported_networks;
    }

    void set_merchant_identifier(std::optional<std::string> value)
    {
        merchant_identifier = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_identifier() const noexcept
    {
        return merchant_identifier;
    }

    void set_required_billing_contact_fields(std::optional<ApplePayBillingContactFields> value)
    {
        required_billing_contact_fields = std::move(value);
    }

    [[nodiscard]] const std::optional<ApplePayBillingContactFields>&
    get_required_billing_contact_fields() const noexcept
    {
        return required_billing_contact_fields;
    }

    void set_required_shipping_contact_fields(std::optional<ApplePayShippingContactFields> value)
    {
        required_shipping_contact_fields = std::move(value);
    }

    [[nodiscard]] const std::optional<ApplePayShippingContactFields>&
    get_required_shipping_contact_fields() const noexcept
    {
        return required_shipping_contact_fields;
    }

    void set_recurring_payment_request(std::optional<ApplePayRecurringPaymentRequest> value)
    {
        recurring_payment_request = std::move(value);
    }

    [[nodiscard]] const std::optional<ApplePayRecurringPaymentRequest>&
    get_recurring_payment_request() const noexcept
    {
        return recurring_payment_request;
    }

    CountryAlpha2Value country_code{};
    CurrencyValue currency_code{};
    AmountInfo total{};
    std::optional<std::vector<std::string>> merchant_capabilities{};
    std::optional<std::vector<std::string>> supported_networks{};
    std::optional<std::string> merchant_identifier{};
    std::optional<ApplePayBillingContactFields> required_billing_contact_fields{};
    std::optional<ApplePayShippingContactFields> required_shipping_contact_fields{};
    std::optional<ApplePayRecurringPaymentRequest> recurring_payment_request{};
};

class ApplepaySessionTokenData : public serde::Serializer<ApplepaySessionTokenData>
{
public:
    void set_apple_pay(ApplePayMetadata value)
    {
        apple_pay = std::move(value);
    }

    [[nodiscard]] const ApplePayMetadata& get_apple_pay() const noexcept
    {
        return apple_pay;
    }

    ApplePayMetadata apple_pay{};
};

class ApplepaySessionTokenResponse : public serde::Serializer<ApplepaySessionTokenResponse>
{
public:
    void set_session_token_data(std::optional<ApplePaySessionResponse> value)
    {
        session_token_data = std::move(value);
    }

    [[nodiscard]] const std::optional<ApplePaySessionResponse>&
    get_session_token_data() const noexcept
    {
        return session_token_data;
    }

    void set_payment_request_data(std::optional<ApplePayPaymentRequest> value)
    {
        payment_request_data = std::move(value);
    }

    [[nodiscard]] const std::optional<ApplePayPaymentRequest>&
    get_payment_request_data() const noexcept
    {
        return payment_request_data;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_delayed_session_token(bool value)
    {
        delayed_session_token = std::move(value);
    }

    [[nodiscard]] const bool& get_delayed_session_token() const noexcept
    {
        return delayed_session_token;
    }

    void set_sdk_next_action(SdkNextAction value)
    {
        sdk_next_action = std::move(value);
    }

    [[nodiscard]] const SdkNextAction& get_sdk_next_action() const noexcept
    {
        return sdk_next_action;
    }

    void set_connector_reference_id(std::optional<std::string> value)
    {
        connector_reference_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_reference_id() const noexcept
    {
        return connector_reference_id;
    }

    void set_connector_sdk_public_key(std::optional<std::string> value)
    {
        connector_sdk_public_key = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_sdk_public_key() const noexcept
    {
        return connector_sdk_public_key;
    }

    void set_connector_merchant_id(std::optional<std::string> value)
    {
        connector_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_connector_merchant_id() const noexcept
    {
        return connector_merchant_id;
    }

    std::optional<ApplePaySessionResponse> session_token_data{};
    std::optional<ApplePayPaymentRequest> payment_request_data{};
    std::string connector{};
    bool delayed_session_token{};
    SdkNextAction sdk_next_action{};
    std::optional<std::string> connector_reference_id{};
    std::optional<std::string> connector_sdk_public_key{};
    std::optional<std::string> connector_merchant_id{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::ApplePayRedirectData>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_redirect_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayThirdPartySdkData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "token",
            &app::model::payments::ApplePayThirdPartySdkData::get_token,
            &app::model::payments::ApplePayThirdPartySdkData::set_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_third_party_sdk_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayWalletData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_data",
                &app::model::payments::ApplePayWalletData::get_payment_data,
                &app::model::payments::ApplePayWalletData::set_payment_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method",
                &app::model::payments::ApplePayWalletData::get_payment_method,
                &app::model::payments::ApplePayWalletData::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_identifier",
                &app::model::payments::ApplePayWalletData::get_transaction_identifier,
                &app::model::payments::ApplePayWalletData::set_transaction_identifier,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_wallet_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplepayPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "display_name",
                &app::model::payments::ApplepayPaymentMethod::get_display_name,
                &app::model::payments::ApplepayPaymentMethod::set_display_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network",
                &app::model::payments::ApplepayPaymentMethod::get_network,
                &app::model::payments::ApplepayPaymentMethod::set_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "type",
                &app::model::payments::ApplepayPaymentMethod::get_pm_type,
                &app::model::payments::ApplepayPaymentMethod::set_pm_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_month",
                &app::model::payments::ApplepayPaymentMethod::get_card_exp_month,
                &app::model::payments::ApplepayPaymentMethod::set_card_exp_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_exp_year",
                &app::model::payments::ApplepayPaymentMethod::get_card_exp_year,
                &app::model::payments::ApplepayPaymentMethod::set_card_exp_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "device_pan_bin",
                &app::model::payments::ApplepayPaymentMethod::get_device_pan_bin,
                &app::model::payments::ApplepayPaymentMethod::set_device_pan_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_bin",
                &app::model::payments::ApplepayPaymentMethod::get_card_bin,
                &app::model::payments::ApplepayPaymentMethod::set_card_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_type",
                &app::model::payments::ApplepayPaymentMethod::get_card_type,
                &app::model::payments::ApplepayPaymentMethod::set_card_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "auth_code",
                &app::model::payments::ApplepayPaymentMethod::get_auth_code,
                &app::model::payments::ApplepayPaymentMethod::set_auth_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_subtype",
                &app::model::payments::ApplepayPaymentMethod::get_card_subtype,
                &app::model::payments::ApplepayPaymentMethod::set_card_subtype,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_segment_type",
                &app::model::payments::ApplepayPaymentMethod::get_card_segment_type,
                &app::model::payments::ApplepayPaymentMethod::set_card_segment_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "funding_source",
                &app::model::payments::ApplepayPaymentMethod::get_funding_source,
                &app::model::payments::ApplepayPaymentMethod::set_funding_source,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_name",
                &app::model::payments::ApplepayPaymentMethod::get_issuer_name,
                &app::model::payments::ApplepayPaymentMethod::set_issuer_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "issuer_country",
                &app::model::payments::ApplepayPaymentMethod::get_issuer_country,
                &app::model::payments::ApplepayPaymentMethod::set_issuer_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_checks",
                &app::model::payments::ApplepayPaymentMethod::get_payment_checks,
                &app::model::payments::ApplepayPaymentMethod::set_payment_checks,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_payment_method";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplepaySessionRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchantIdentifier",
                &app::model::payments::ApplepaySessionRequest::get_merchant_identifier,
                &app::model::payments::ApplepaySessionRequest::set_merchant_identifier,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "displayName",
                &app::model::payments::ApplepaySessionRequest::get_display_name,
                &app::model::payments::ApplepaySessionRequest::set_display_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "initiative",
                &app::model::payments::ApplepaySessionRequest::get_initiative,
                &app::model::payments::ApplepaySessionRequest::set_initiative,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "initiativeContext",
                &app::model::payments::ApplepaySessionRequest::get_initiative_context,
                &app::model::payments::ApplepaySessionRequest::set_initiative_context,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_session_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplepaySessionTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "apple_pay",
            &app::model::payments::ApplepaySessionTokenData::get_apple_pay,
            &app::model::payments::ApplepaySessionTokenData::set_apple_pay,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_session_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayCombinedWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data",
                &app::model::payments::ApplePayCombinedWrapper::get_data,
                &app::model::payments::ApplePayCombinedWrapper::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "support_predecrypted_token",
                &app::model::payments::ApplePayCombinedWrapper::get_support_predecrypted_token,
                &app::model::payments::ApplePayCombinedWrapper::set_support_predecrypted_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_combined_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplepayCombinedSessionTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "apple_pay_combined",
            &app::model::payments::ApplepayCombinedSessionTokenData::get_apple_pay_combined,
            &app::model::payments::ApplepayCombinedSessionTokenData::set_apple_pay_combined,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_combined_session_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayMetadata>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_request_data",
                &app::model::payments::ApplePayMetadata::get_payment_request_data,
                &app::model::payments::ApplePayMetadata::set_payment_request_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_token_data",
                &app::model::payments::ApplePayMetadata::get_session_token_data,
                &app::model::payments::ApplePayMetadata::set_session_token_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_metadata";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplepaySessionTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "session_token_data",
                &app::model::payments::ApplepaySessionTokenResponse::get_session_token_data,
                &app::model::payments::ApplepaySessionTokenResponse::set_session_token_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_request_data",
                &app::model::payments::ApplepaySessionTokenResponse::get_payment_request_data,
                &app::model::payments::ApplepaySessionTokenResponse::set_payment_request_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::ApplepaySessionTokenResponse::get_connector,
                &app::model::payments::ApplepaySessionTokenResponse::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delayed_session_token",
                &app::model::payments::ApplepaySessionTokenResponse::get_delayed_session_token,
                &app::model::payments::ApplepaySessionTokenResponse::set_delayed_session_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_next_action",
                &app::model::payments::ApplepaySessionTokenResponse::get_sdk_next_action,
                &app::model::payments::ApplepaySessionTokenResponse::set_sdk_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_reference_id",
                &app::model::payments::ApplepaySessionTokenResponse::get_connector_reference_id,
                &app::model::payments::ApplepaySessionTokenResponse::set_connector_reference_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_sdk_public_key",
                &app::model::payments::ApplepaySessionTokenResponse::get_connector_sdk_public_key,
                &app::model::payments::ApplepaySessionTokenResponse::set_connector_sdk_public_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_merchant_id",
                &app::model::payments::ApplepaySessionTokenResponse::get_connector_merchant_id,
                &app::model::payments::ApplepaySessionTokenResponse::set_connector_merchant_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_session_token_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayPaymentRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "country_code",
                &app::model::payments::ApplePayPaymentRequest::get_country_code,
                &app::model::payments::ApplePayPaymentRequest::set_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency_code",
                &app::model::payments::ApplePayPaymentRequest::get_currency_code,
                &app::model::payments::ApplePayPaymentRequest::set_currency_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total",
                &app::model::payments::ApplePayPaymentRequest::get_total,
                &app::model::payments::ApplePayPaymentRequest::set_total,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_capabilities",
                &app::model::payments::ApplePayPaymentRequest::get_merchant_capabilities,
                &app::model::payments::ApplePayPaymentRequest::set_merchant_capabilities,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "supported_networks",
                &app::model::payments::ApplePayPaymentRequest::get_supported_networks,
                &app::model::payments::ApplePayPaymentRequest::set_supported_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_identifier",
                &app::model::payments::ApplePayPaymentRequest::get_merchant_identifier,
                &app::model::payments::ApplePayPaymentRequest::set_merchant_identifier,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "required_billing_contact_fields",
                &app::model::payments::ApplePayPaymentRequest::get_required_billing_contact_fields,
                &app::model::payments::ApplePayPaymentRequest::set_required_billing_contact_fields,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "required_shipping_contact_fields",
                &app::model::payments::ApplePayPaymentRequest::get_required_shipping_contact_fields,
                &app::model::payments::ApplePayPaymentRequest::set_required_shipping_contact_fields,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_request",
                &app::model::payments::ApplePayPaymentRequest::get_recurring_payment_request,
                &app::model::payments::ApplePayPaymentRequest::set_recurring_payment_request,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_payment_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayRecurringPaymentRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_description",
                &app::model::payments::ApplePayRecurringPaymentRequest::get_payment_description,
                &app::model::payments::ApplePayRecurringPaymentRequest::set_payment_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "regular_billing",
                &app::model::payments::ApplePayRecurringPaymentRequest::get_regular_billing,
                &app::model::payments::ApplePayRecurringPaymentRequest::set_regular_billing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_agreement",
                &app::model::payments::ApplePayRecurringPaymentRequest::get_billing_agreement,
                &app::model::payments::ApplePayRecurringPaymentRequest::set_billing_agreement,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "management_u_r_l",
                &app::model::payments::ApplePayRecurringPaymentRequest::get_management_u_r_l,
                &app::model::payments::ApplePayRecurringPaymentRequest::set_management_u_r_l,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_recurring_payment_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayRegularBillingRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::payments::ApplePayRegularBillingRequest::get_amount,
                &app::model::payments::ApplePayRegularBillingRequest::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "label",
                &app::model::payments::ApplePayRegularBillingRequest::get_label,
                &app::model::payments::ApplePayRegularBillingRequest::set_label,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_timing",
                &app::model::payments::ApplePayRegularBillingRequest::get_payment_timing,
                &app::model::payments::ApplePayRegularBillingRequest::set_payment_timing,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_start_date",
                &app::model::payments::ApplePayRegularBillingRequest::
                    get_recurring_payment_start_date,
                &app::model::payments::ApplePayRegularBillingRequest::
                    set_recurring_payment_start_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_end_date",
                &app::model::payments::ApplePayRegularBillingRequest::
                    get_recurring_payment_end_date,
                &app::model::payments::ApplePayRegularBillingRequest::
                    set_recurring_payment_end_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_interval_unit",
                &app::model::payments::ApplePayRegularBillingRequest::
                    get_recurring_payment_interval_unit,
                &app::model::payments::ApplePayRegularBillingRequest::
                    set_recurring_payment_interval_unit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment_interval_count",
                &app::model::payments::ApplePayRegularBillingRequest::
                    get_recurring_payment_interval_count,
                &app::model::payments::ApplePayRegularBillingRequest::
                    set_recurring_payment_interval_count,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_regular_billing_request";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayBillingContactFields>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::model::payments::ApplePayBillingContactFields::get_value,
            &app::model::payments::ApplePayBillingContactFields::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_billing_contact_fields";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplePayShippingContactFields>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "value",
            &app::model::payments::ApplePayShippingContactFields::get_value,
            &app::model::payments::ApplePayShippingContactFields::set_value,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_shipping_contact_fields";
    }
};

template<>
struct serde::Serializable<app::model::payments::ApplepayErrorResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "statusCode",
                &app::model::payments::ApplepayErrorResponse::get_status_code,
                &app::model::payments::ApplepayErrorResponse::set_status_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "statusMessage",
                &app::model::payments::ApplepayErrorResponse::get_status_message,
                &app::model::payments::ApplepayErrorResponse::set_status_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "applepay_error_response";
    }
};
