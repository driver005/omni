module;
export module app_connector:model_googlepay_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class GooglePayAssuranceDetails : public serde::Serializer<GooglePayAssuranceDetails>
{
public:
    void set_card_holder_authenticated(bool value)
    {
        card_holder_authenticated = std::move(value);
    }

    [[nodiscard]] const bool& get_card_holder_authenticated() const noexcept
    {
        return card_holder_authenticated;
    }

    void set_account_verified(bool value)
    {
        account_verified = std::move(value);
    }

    [[nodiscard]] const bool& get_account_verified() const noexcept
    {
        return account_verified;
    }

    bool card_holder_authenticated{};
    bool account_verified{};
};

class GooglePayCardFundingSourceValue : public serde::Serializer<GooglePayCardFundingSourceValue>
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

class GooglePayRedirectData : public serde::Serializer<GooglePayRedirectData>
{
public:
};

enum class GooglePayTokenizationSpecificationType
{
    PaymentGateway,
    Direct,
};

enum class GooglePayTokenizationType
{
    PaymentGateway,
    Direct,
    InternalGateway,
};

enum class GpayBillingAddressFormat
{
    FULL,
    MIN,
};

class GpayMerchantInfo : public serde::Serializer<GpayMerchantInfo>
{
public:
    void set_merchant_id(std::optional<std::string> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_merchant_name(std::string value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    std::optional<std::string> merchant_id{};
    std::string merchant_name{};
};

class GpayShippingAddressParameters : public serde::Serializer<GpayShippingAddressParameters>
{
public:
    void set_phone_number_required(bool value)
    {
        phone_number_required = std::move(value);
    }

    [[nodiscard]] const bool& get_phone_number_required() const noexcept
    {
        return phone_number_required;
    }

    bool phone_number_required{};
};

class GpayTokenizationDataValue : public serde::Serializer<GpayTokenizationDataValue>
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

class GooglePayPaymentMethodInfo : public serde::Serializer<GooglePayPaymentMethodInfo>
{
public:
    void set_card_network(std::string value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_network() const noexcept
    {
        return card_network;
    }

    void set_card_details(std::string value)
    {
        card_details = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_details() const noexcept
    {
        return card_details;
    }

    void set_assurance_details(std::optional<GooglePayAssuranceDetails> value)
    {
        assurance_details = std::move(value);
    }

    [[nodiscard]] const std::optional<GooglePayAssuranceDetails>&
    get_assurance_details() const noexcept
    {
        return assurance_details;
    }

    void set_card_funding_source(std::optional<GooglePayCardFundingSourceValue> value)
    {
        card_funding_source = std::move(value);
    }

    [[nodiscard]] const std::optional<GooglePayCardFundingSourceValue>&
    get_card_funding_source() const noexcept
    {
        return card_funding_source;
    }

    std::string card_network{};
    std::string card_details{};
    std::optional<GooglePayAssuranceDetails> assurance_details{};
    std::optional<GooglePayCardFundingSourceValue> card_funding_source{};
};

class GooglePayThirdPartySdkData : public serde::Serializer<GooglePayThirdPartySdkData>
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

class GooglePayTokenizationParameters : public serde::Serializer<GooglePayTokenizationParameters>
{
public:
    void set_gateway(std::optional<std::string> value)
    {
        gateway = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_gateway() const noexcept
    {
        return gateway;
    }

    void set_public_key(std::optional<SecretValue> value)
    {
        public_key = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_public_key() const noexcept
    {
        return public_key;
    }

    void set_private_key(std::optional<SecretValue> value)
    {
        private_key = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_private_key() const noexcept
    {
        return private_key;
    }

    void set_recipient_id(std::optional<SecretValue> value)
    {
        recipient_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_recipient_id() const noexcept
    {
        return recipient_id;
    }

    void set_gateway_merchant_id(std::optional<SecretValue> value)
    {
        gateway_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_gateway_merchant_id() const noexcept
    {
        return gateway_merchant_id;
    }

    void set_stripe_publishable_key(std::optional<SecretValue> value)
    {
        stripe_publishable_key = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_stripe_publishable_key() const noexcept
    {
        return stripe_publishable_key;
    }

    void set_stripe_version(std::optional<SecretValue> value)
    {
        stripe_version = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_stripe_version() const noexcept
    {
        return stripe_version;
    }

    std::optional<std::string> gateway{};
    std::optional<SecretValue> public_key{};
    std::optional<SecretValue> private_key{};
    std::optional<SecretValue> recipient_id{};
    std::optional<SecretValue> gateway_merchant_id{};
    std::optional<SecretValue> stripe_publishable_key{};
    std::optional<SecretValue> stripe_version{};
};

class GpayBillingAddressParameters : public serde::Serializer<GpayBillingAddressParameters>
{
public:
    void set_phone_number_required(bool value)
    {
        phone_number_required = std::move(value);
    }

    [[nodiscard]] const bool& get_phone_number_required() const noexcept
    {
        return phone_number_required;
    }

    void set_format(GpayBillingAddressFormat value)
    {
        format = std::move(value);
    }

    [[nodiscard]] const GpayBillingAddressFormat& get_format() const noexcept
    {
        return format;
    }

    bool phone_number_required{};
    GpayBillingAddressFormat format{};
};

class GpayTokenParameters : public serde::Serializer<GpayTokenParameters>
{
public:
    void set_gateway(std::optional<std::string> value)
    {
        gateway = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_gateway() const noexcept
    {
        return gateway;
    }

    void set_gateway_merchant_id(std::optional<std::string> value)
    {
        gateway_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_gateway_merchant_id() const noexcept
    {
        return gateway_merchant_id;
    }

    void set_stripe_version(std::optional<std::string> value)
    {
        stripe_version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_stripe_version() const noexcept
    {
        return stripe_version;
    }

    void set_stripe_publishable_key(std::optional<std::string> value)
    {
        stripe_publishable_key = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_stripe_publishable_key() const noexcept
    {
        return stripe_publishable_key;
    }

    void set_protocol_version(std::optional<std::string> value)
    {
        protocol_version = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_protocol_version() const noexcept
    {
        return protocol_version;
    }

    void set_public_key(std::optional<SecretValue> value)
    {
        public_key = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_public_key() const noexcept
    {
        return public_key;
    }

    std::optional<std::string> gateway{};
    std::optional<std::string> gateway_merchant_id{};
    std::optional<std::string> stripe_version{};
    std::optional<std::string> stripe_publishable_key{};
    std::optional<std::string> protocol_version{};
    std::optional<SecretValue> public_key{};
};

class GpayTransactionInfo : public serde::Serializer<GpayTransactionInfo>
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

    void set_total_price_status(std::string value)
    {
        total_price_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_total_price_status() const noexcept
    {
        return total_price_status;
    }

    void set_total_price(StringMajorUnitValue value)
    {
        total_price = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_total_price() const noexcept
    {
        return total_price;
    }

    CountryAlpha2Value country_code{};
    CurrencyValue currency_code{};
    std::string total_price_status{};
    StringMajorUnitValue total_price{};
};

class GooglePayTokenizationSpecification :
    public serde::Serializer<GooglePayTokenizationSpecification>
{
public:
    void set_tokenization_type(GooglePayTokenizationType value)
    {
        tokenization_type = std::move(value);
    }

    [[nodiscard]] const GooglePayTokenizationType& get_tokenization_type() const noexcept
    {
        return tokenization_type;
    }

    void set_parameters(GooglePayTokenizationParameters value)
    {
        parameters = std::move(value);
    }

    [[nodiscard]] const GooglePayTokenizationParameters& get_parameters() const noexcept
    {
        return parameters;
    }

    GooglePayTokenizationType tokenization_type{};
    GooglePayTokenizationParameters parameters{};
};

class GooglePayWalletData : public serde::Serializer<GooglePayWalletData>
{
public:
    void set_pm_type(std::string value)
    {
        pm_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_pm_type() const noexcept
    {
        return pm_type;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_info(GooglePayPaymentMethodInfo value)
    {
        info = std::move(value);
    }

    [[nodiscard]] const GooglePayPaymentMethodInfo& get_info() const noexcept
    {
        return info;
    }

    void set_tokenization_data(GpayTokenizationDataValue value)
    {
        tokenization_data = std::move(value);
    }

    [[nodiscard]] const GpayTokenizationDataValue& get_tokenization_data() const noexcept
    {
        return tokenization_data;
    }

    std::string pm_type{};
    std::string description{};
    GooglePayPaymentMethodInfo info{};
    GpayTokenizationDataValue tokenization_data{};
};

class GpayAllowedMethodsParameters : public serde::Serializer<GpayAllowedMethodsParameters>
{
public:
    void set_allowed_auth_methods(std::vector<std::string> value)
    {
        allowed_auth_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_allowed_auth_methods() const noexcept
    {
        return allowed_auth_methods;
    }

    void set_allowed_card_networks(std::vector<std::string> value)
    {
        allowed_card_networks = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_allowed_card_networks() const noexcept
    {
        return allowed_card_networks;
    }

    void set_billing_address_required(std::optional<bool> value)
    {
        billing_address_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_billing_address_required() const noexcept
    {
        return billing_address_required;
    }

    void set_billing_address_parameters(std::optional<GpayBillingAddressParameters> value)
    {
        billing_address_parameters = std::move(value);
    }

    [[nodiscard]] const std::optional<GpayBillingAddressParameters>&
    get_billing_address_parameters() const noexcept
    {
        return billing_address_parameters;
    }

    void set_assurance_details_required(std::optional<bool> value)
    {
        assurance_details_required = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_assurance_details_required() const noexcept
    {
        return assurance_details_required;
    }

    void set_allow_credit_cards(std::optional<bool> value)
    {
        allow_credit_cards = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_allow_credit_cards() const noexcept
    {
        return allow_credit_cards;
    }

    std::vector<std::string> allowed_auth_methods{};
    std::vector<std::string> allowed_card_networks{};
    std::optional<bool> billing_address_required{};
    std::optional<GpayBillingAddressParameters> billing_address_parameters{};
    std::optional<bool> assurance_details_required{};
    std::optional<bool> allow_credit_cards{};
};

class GpayTokenizationSpecification : public serde::Serializer<GpayTokenizationSpecification>
{
public:
    void set_token_specification_type(GooglePayTokenizationSpecificationType value)
    {
        token_specification_type = std::move(value);
    }

    [[nodiscard]] const GooglePayTokenizationSpecificationType&
    get_token_specification_type() const noexcept
    {
        return token_specification_type;
    }

    void set_parameters(GpayTokenParameters value)
    {
        parameters = std::move(value);
    }

    [[nodiscard]] const GpayTokenParameters& get_parameters() const noexcept
    {
        return parameters;
    }

    GooglePayTokenizationSpecificationType token_specification_type{};
    GpayTokenParameters parameters{};
};

class GooglePayMerchantInfo : public serde::Serializer<GooglePayMerchantInfo>
{
public:
    void set_merchant_name(std::string value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_merchant_id(std::optional<std::string> value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_tokenization_specification(GooglePayTokenizationSpecification value)
    {
        tokenization_specification = std::move(value);
    }

    [[nodiscard]] const GooglePayTokenizationSpecification&
    get_tokenization_specification() const noexcept
    {
        return tokenization_specification;
    }

    std::string merchant_name{};
    std::optional<std::string> merchant_id{};
    GooglePayTokenizationSpecification tokenization_specification{};
};

class GpayAllowedPaymentMethods : public serde::Serializer<GpayAllowedPaymentMethods>
{
public:
    void set_payment_method_type(std::string value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_parameters(GpayAllowedMethodsParameters value)
    {
        parameters = std::move(value);
    }

    [[nodiscard]] const GpayAllowedMethodsParameters& get_parameters() const noexcept
    {
        return parameters;
    }

    void set_tokenization_specification(GpayTokenizationSpecification value)
    {
        tokenization_specification = std::move(value);
    }

    [[nodiscard]] const GpayTokenizationSpecification&
    get_tokenization_specification() const noexcept
    {
        return tokenization_specification;
    }

    std::string payment_method_type{};
    GpayAllowedMethodsParameters parameters{};
    GpayTokenizationSpecification tokenization_specification{};
};

class GooglePayMerchantDetails : public serde::Serializer<GooglePayMerchantDetails>
{
public:
    void set_merchant_info(GooglePayMerchantInfo value)
    {
        merchant_info = std::move(value);
    }

    [[nodiscard]] const GooglePayMerchantInfo& get_merchant_info() const noexcept
    {
        return merchant_info;
    }

    GooglePayMerchantInfo merchant_info{};
};

class GooglePaySessionResponse : public serde::Serializer<GooglePaySessionResponse>
{
public:
    void set_merchant_info(GpayMerchantInfo value)
    {
        merchant_info = std::move(value);
    }

    [[nodiscard]] const GpayMerchantInfo& get_merchant_info() const noexcept
    {
        return merchant_info;
    }

    void set_shipping_address_required(bool value)
    {
        shipping_address_required = std::move(value);
    }

    [[nodiscard]] const bool& get_shipping_address_required() const noexcept
    {
        return shipping_address_required;
    }

    void set_email_required(bool value)
    {
        email_required = std::move(value);
    }

    [[nodiscard]] const bool& get_email_required() const noexcept
    {
        return email_required;
    }

    void set_shipping_address_parameters(GpayShippingAddressParameters value)
    {
        shipping_address_parameters = std::move(value);
    }

    [[nodiscard]] const GpayShippingAddressParameters&
    get_shipping_address_parameters() const noexcept
    {
        return shipping_address_parameters;
    }

    void set_allowed_payment_methods(std::vector<GpayAllowedPaymentMethods> value)
    {
        allowed_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<GpayAllowedPaymentMethods>&
    get_allowed_payment_methods() const noexcept
    {
        return allowed_payment_methods;
    }

    void set_transaction_info(GpayTransactionInfo value)
    {
        transaction_info = std::move(value);
    }

    [[nodiscard]] const GpayTransactionInfo& get_transaction_info() const noexcept
    {
        return transaction_info;
    }

    void set_delayed_session_token(bool value)
    {
        delayed_session_token = std::move(value);
    }

    [[nodiscard]] const bool& get_delayed_session_token() const noexcept
    {
        return delayed_session_token;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_sdk_next_action(SdkNextAction value)
    {
        sdk_next_action = std::move(value);
    }

    [[nodiscard]] const SdkNextAction& get_sdk_next_action() const noexcept
    {
        return sdk_next_action;
    }

    void set_secrets(std::optional<SecretInfoToInitiateSdk> value)
    {
        secrets = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretInfoToInitiateSdk>& get_secrets() const noexcept
    {
        return secrets;
    }

    GpayMerchantInfo merchant_info{};
    bool shipping_address_required{};
    bool email_required{};
    GpayShippingAddressParameters shipping_address_parameters{};
    std::vector<GpayAllowedPaymentMethods> allowed_payment_methods{};
    GpayTransactionInfo transaction_info{};
    bool delayed_session_token{};
    std::string connector{};
    SdkNextAction sdk_next_action{};
    std::optional<SecretInfoToInitiateSdk> secrets{};
};

class GpayMetaData : public serde::Serializer<GpayMetaData>
{
public:
    void set_merchant_info(GpayMerchantInfo value)
    {
        merchant_info = std::move(value);
    }

    [[nodiscard]] const GpayMerchantInfo& get_merchant_info() const noexcept
    {
        return merchant_info;
    }

    void set_allowed_payment_methods(std::vector<GpayAllowedPaymentMethods> value)
    {
        allowed_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<GpayAllowedPaymentMethods>&
    get_allowed_payment_methods() const noexcept
    {
        return allowed_payment_methods;
    }

    GpayMerchantInfo merchant_info{};
    std::vector<GpayAllowedPaymentMethods> allowed_payment_methods{};
};

class GooglePayDetailsWrapper : public serde::Serializer<GooglePayDetailsWrapper>
{
public:
    void set_data(std::optional<GpayMetaData> value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const std::optional<GpayMetaData>& get_data() const noexcept
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

    std::optional<GpayMetaData> data{};
    std::optional<bool> support_predecrypted_token{};
};

enum class GooglePayProviderDetails
{
    GooglePayMerchantDetails,
};

class GooglePayDetails : public serde::Serializer<GooglePayDetails>
{
public:
    void set_provider_details(GooglePayProviderDetails value)
    {
        provider_details = std::move(value);
    }

    [[nodiscard]] const GooglePayProviderDetails& get_provider_details() const noexcept
    {
        return provider_details;
    }

    void set_cards(GpayAllowedMethodsParameters value)
    {
        cards = std::move(value);
    }

    [[nodiscard]] const GpayAllowedMethodsParameters& get_cards() const noexcept
    {
        return cards;
    }

    GooglePayProviderDetails provider_details{};
    GpayAllowedMethodsParameters cards{};
};

class GooglePayWalletDetails : public serde::Serializer<GooglePayWalletDetails>
{
public:
    void set_google_pay(GooglePayDetails value)
    {
        google_pay = std::move(value);
    }

    [[nodiscard]] const GooglePayDetails& get_google_pay() const noexcept
    {
        return google_pay;
    }

    GooglePayDetails google_pay{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::GooglePayWalletData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "type",
                &app::model::payments::GooglePayWalletData::get_pm_type,
                &app::model::payments::GooglePayWalletData::set_pm_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::model::payments::GooglePayWalletData::get_description,
                &app::model::payments::GooglePayWalletData::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "info",
                &app::model::payments::GooglePayWalletData::get_info,
                &app::model::payments::GooglePayWalletData::set_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization_data",
                &app::model::payments::GooglePayWalletData::get_tokenization_data,
                &app::model::payments::GooglePayWalletData::set_tokenization_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_wallet_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayRedirectData>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_redirect_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayThirdPartySdkData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "token",
            &app::model::payments::GooglePayThirdPartySdkData::get_token,
            &app::model::payments::GooglePayThirdPartySdkData::set_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_third_party_sdk_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayPaymentMethodInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_network",
                &app::model::payments::GooglePayPaymentMethodInfo::get_card_network,
                &app::model::payments::GooglePayPaymentMethodInfo::set_card_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_details",
                &app::model::payments::GooglePayPaymentMethodInfo::get_card_details,
                &app::model::payments::GooglePayPaymentMethodInfo::set_card_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "assurance_details",
                &app::model::payments::GooglePayPaymentMethodInfo::get_assurance_details,
                &app::model::payments::GooglePayPaymentMethodInfo::set_assurance_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_funding_source",
                &app::model::payments::GooglePayPaymentMethodInfo::get_card_funding_source,
                &app::model::payments::GooglePayPaymentMethodInfo::set_card_funding_source,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_payment_method_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayAssuranceDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "card_holder_authenticated",
                &app::model::payments::GooglePayAssuranceDetails::get_card_holder_authenticated,
                &app::model::payments::GooglePayAssuranceDetails::set_card_holder_authenticated,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_verified",
                &app::model::payments::GooglePayAssuranceDetails::get_account_verified,
                &app::model::payments::GooglePayAssuranceDetails::set_account_verified,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_assurance_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayAllowedMethodsParameters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "allowed_auth_methods",
                &app::model::payments::GpayAllowedMethodsParameters::get_allowed_auth_methods,
                &app::model::payments::GpayAllowedMethodsParameters::set_allowed_auth_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allowed_card_networks",
                &app::model::payments::GpayAllowedMethodsParameters::get_allowed_card_networks,
                &app::model::payments::GpayAllowedMethodsParameters::set_allowed_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_required",
                &app::model::payments::GpayAllowedMethodsParameters::get_billing_address_required,
                &app::model::payments::GpayAllowedMethodsParameters::set_billing_address_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_parameters",
                &app::model::payments::GpayAllowedMethodsParameters::get_billing_address_parameters,
                &app::model::payments::GpayAllowedMethodsParameters::set_billing_address_parameters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "assurance_details_required",
                &app::model::payments::GpayAllowedMethodsParameters::get_assurance_details_required,
                &app::model::payments::GpayAllowedMethodsParameters::set_assurance_details_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allow_credit_cards",
                &app::model::payments::GpayAllowedMethodsParameters::get_allow_credit_cards,
                &app::model::payments::GpayAllowedMethodsParameters::set_allow_credit_cards,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_allowed_methods_parameters";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayBillingAddressParameters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "phone_number_required",
                &app::model::payments::GpayBillingAddressParameters::get_phone_number_required,
                &app::model::payments::GpayBillingAddressParameters::set_phone_number_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "format",
                &app::model::payments::GpayBillingAddressParameters::get_format,
                &app::model::payments::GpayBillingAddressParameters::set_format,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_billing_address_parameters";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayTokenParameters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "gateway",
                &app::model::payments::GpayTokenParameters::get_gateway,
                &app::model::payments::GpayTokenParameters::set_gateway,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_merchant_id",
                &app::model::payments::GpayTokenParameters::get_gateway_merchant_id,
                &app::model::payments::GpayTokenParameters::set_gateway_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "stripe:version",
                &app::model::payments::GpayTokenParameters::get_stripe_version,
                &app::model::payments::GpayTokenParameters::set_stripe_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "stripe:publishableKey",
                &app::model::payments::GpayTokenParameters::get_stripe_publishable_key,
                &app::model::payments::GpayTokenParameters::set_stripe_publishable_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "protocol_version",
                &app::model::payments::GpayTokenParameters::get_protocol_version,
                &app::model::payments::GpayTokenParameters::set_protocol_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "public_key",
                &app::model::payments::GpayTokenParameters::get_public_key,
                &app::model::payments::GpayTokenParameters::set_public_key,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_token_parameters";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayTokenizationSpecification>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "type",
                &app::model::payments::GpayTokenizationSpecification::get_token_specification_type,
                &app::model::payments::GpayTokenizationSpecification::set_token_specification_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parameters",
                &app::model::payments::GpayTokenizationSpecification::get_parameters,
                &app::model::payments::GpayTokenizationSpecification::set_parameters,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_tokenization_specification";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayAllowedPaymentMethods>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "type",
                &app::model::payments::GpayAllowedPaymentMethods::get_payment_method_type,
                &app::model::payments::GpayAllowedPaymentMethods::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parameters",
                &app::model::payments::GpayAllowedPaymentMethods::get_parameters,
                &app::model::payments::GpayAllowedPaymentMethods::set_parameters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization_specification",
                &app::model::payments::GpayAllowedPaymentMethods::get_tokenization_specification,
                &app::model::payments::GpayAllowedPaymentMethods::set_tokenization_specification,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_allowed_payment_methods";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayTransactionInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "country_code",
                &app::model::payments::GpayTransactionInfo::get_country_code,
                &app::model::payments::GpayTransactionInfo::set_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency_code",
                &app::model::payments::GpayTransactionInfo::get_currency_code,
                &app::model::payments::GpayTransactionInfo::set_currency_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_price_status",
                &app::model::payments::GpayTransactionInfo::get_total_price_status,
                &app::model::payments::GpayTransactionInfo::set_total_price_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_price",
                &app::model::payments::GpayTransactionInfo::get_total_price,
                &app::model::payments::GpayTransactionInfo::set_total_price,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_transaction_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayMerchantInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::GpayMerchantInfo::get_merchant_id,
                &app::model::payments::GpayMerchantInfo::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::model::payments::GpayMerchantInfo::get_merchant_name,
                &app::model::payments::GpayMerchantInfo::set_merchant_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_merchant_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayMetaData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_info",
                &app::model::payments::GpayMetaData::get_merchant_info,
                &app::model::payments::GpayMetaData::set_merchant_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allowed_payment_methods",
                &app::model::payments::GpayMetaData::get_allowed_payment_methods,
                &app::model::payments::GpayMetaData::set_allowed_payment_methods,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_meta_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayDetailsWrapper>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "data",
                &app::model::payments::GooglePayDetailsWrapper::get_data,
                &app::model::payments::GooglePayDetailsWrapper::set_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "support_predecrypted_token",
                &app::model::payments::GooglePayDetailsWrapper::get_support_predecrypted_token,
                &app::model::payments::GooglePayDetailsWrapper::set_support_predecrypted_token,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_details_wrapper";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayWalletDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "google_pay",
            &app::model::payments::GooglePayWalletDetails::get_google_pay,
            &app::model::payments::GooglePayWalletDetails::set_google_pay,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_wallet_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "provider_details",
                &app::model::payments::GooglePayDetails::get_provider_details,
                &app::model::payments::GooglePayDetails::set_provider_details,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cards",
                &app::model::payments::GooglePayDetails::get_cards,
                &app::model::payments::GooglePayDetails::set_cards,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayMerchantDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "merchant_info",
            &app::model::payments::GooglePayMerchantDetails::get_merchant_info,
            &app::model::payments::GooglePayMerchantDetails::set_merchant_info,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_merchant_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayMerchantInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_name",
                &app::model::payments::GooglePayMerchantInfo::get_merchant_name,
                &app::model::payments::GooglePayMerchantInfo::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::GooglePayMerchantInfo::get_merchant_id,
                &app::model::payments::GooglePayMerchantInfo::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tokenization_specification",
                &app::model::payments::GooglePayMerchantInfo::get_tokenization_specification,
                &app::model::payments::GooglePayMerchantInfo::set_tokenization_specification,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_merchant_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayTokenizationSpecification>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "type",
                &app::model::payments::GooglePayTokenizationSpecification::get_tokenization_type,
                &app::model::payments::GooglePayTokenizationSpecification::set_tokenization_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parameters",
                &app::model::payments::GooglePayTokenizationSpecification::get_parameters,
                &app::model::payments::GooglePayTokenizationSpecification::set_parameters,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_tokenization_specification";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePayTokenizationParameters>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "gateway",
                &app::model::payments::GooglePayTokenizationParameters::get_gateway,
                &app::model::payments::GooglePayTokenizationParameters::set_gateway,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "public_key",
                &app::model::payments::GooglePayTokenizationParameters::get_public_key,
                &app::model::payments::GooglePayTokenizationParameters::set_public_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "private_key",
                &app::model::payments::GooglePayTokenizationParameters::get_private_key,
                &app::model::payments::GooglePayTokenizationParameters::set_private_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recipient_id",
                &app::model::payments::GooglePayTokenizationParameters::get_recipient_id,
                &app::model::payments::GooglePayTokenizationParameters::set_recipient_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "gateway_merchant_id",
                &app::model::payments::GooglePayTokenizationParameters::get_gateway_merchant_id,
                &app::model::payments::GooglePayTokenizationParameters::set_gateway_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "stripe:publishableKey",
                &app::model::payments::GooglePayTokenizationParameters::get_stripe_publishable_key,
                &app::model::payments::GooglePayTokenizationParameters::set_stripe_publishable_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "stripe:version",
                &app::model::payments::GooglePayTokenizationParameters::get_stripe_version,
                &app::model::payments::GooglePayTokenizationParameters::set_stripe_version,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_tokenization_parameters";
    }
};

template<>
struct serde::Serializable<app::model::payments::GooglePaySessionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_info",
                &app::model::payments::GooglePaySessionResponse::get_merchant_info,
                &app::model::payments::GooglePaySessionResponse::set_merchant_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_address_required",
                &app::model::payments::GooglePaySessionResponse::get_shipping_address_required,
                &app::model::payments::GooglePaySessionResponse::set_shipping_address_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email_required",
                &app::model::payments::GooglePaySessionResponse::get_email_required,
                &app::model::payments::GooglePaySessionResponse::set_email_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_address_parameters",
                &app::model::payments::GooglePaySessionResponse::get_shipping_address_parameters,
                &app::model::payments::GooglePaySessionResponse::set_shipping_address_parameters,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allowed_payment_methods",
                &app::model::payments::GooglePaySessionResponse::get_allowed_payment_methods,
                &app::model::payments::GooglePaySessionResponse::set_allowed_payment_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_info",
                &app::model::payments::GooglePaySessionResponse::get_transaction_info,
                &app::model::payments::GooglePaySessionResponse::set_transaction_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delayed_session_token",
                &app::model::payments::GooglePaySessionResponse::get_delayed_session_token,
                &app::model::payments::GooglePaySessionResponse::set_delayed_session_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::model::payments::GooglePaySessionResponse::get_connector,
                &app::model::payments::GooglePaySessionResponse::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_next_action",
                &app::model::payments::GooglePaySessionResponse::get_sdk_next_action,
                &app::model::payments::GooglePaySessionResponse::set_sdk_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secrets",
                &app::model::payments::GooglePaySessionResponse::get_secrets,
                &app::model::payments::GooglePaySessionResponse::set_secrets,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_session_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::GpayShippingAddressParameters>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "phone_number_required",
            &app::model::payments::GpayShippingAddressParameters::get_phone_number_required,
            &app::model::payments::GpayShippingAddressParameters::set_phone_number_required,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "gpay_shipping_address_parameters";
    }
};
