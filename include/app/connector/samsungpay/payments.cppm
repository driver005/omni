module;
export module app_connector:model_samsungpay_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

enum class SamsungPayAmountFormat
{
    FormatTotalPriceOnly,
    FormatTotalEstimatedAmount,
};

enum class SamsungPayCardBrand
{
    Visa,
    MasterCard,
    Amex,
    Discover,
    Unknown,
};

enum class SamsungPayCombinedMetadata
{
    ApplicationCredentials,
    MerchantCredentials,
};

enum class SamsungPayProtocolType
{
    Protocol3ds,
};

enum class SamsungPayWalletCredentials
{
    SamsungPayWalletDataForWeb,
    SamsungPayWalletDataForApp,
};

class SamsungPayAmountDetails : public serde::Serializer<SamsungPayAmountDetails>
{
public:
    void set_amount_format(SamsungPayAmountFormat value)
    {
        amount_format = std::move(value);
    }

    [[nodiscard]] const SamsungPayAmountFormat& get_amount_format() const noexcept
    {
        return amount_format;
    }

    void set_currency_code(CurrencyValue value)
    {
        currency_code = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency_code() const noexcept
    {
        return currency_code;
    }

    void set_total_amount(StringMajorUnitValue value)
    {
        total_amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_total_amount() const noexcept
    {
        return total_amount;
    }

    SamsungPayAmountFormat amount_format{};
    CurrencyValue currency_code{};
    StringMajorUnitValue total_amount{};
};

class SamsungPayApplicationCredentials : public serde::Serializer<SamsungPayApplicationCredentials>
{
public:
    void set_merchant_display_name(std::string value)
    {
        merchant_display_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_display_name() const noexcept
    {
        return merchant_display_name;
    }

    void set_merchant_business_country(CountryAlpha2Value value)
    {
        merchant_business_country = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_merchant_business_country() const noexcept
    {
        return merchant_business_country;
    }

    void set_allowed_brands(std::vector<std::string> value)
    {
        allowed_brands = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_allowed_brands() const noexcept
    {
        return allowed_brands;
    }

    std::string merchant_display_name{};
    CountryAlpha2Value merchant_business_country{};
    std::vector<std::string> allowed_brands{};
};

class SamsungPayMerchantCredentials : public serde::Serializer<SamsungPayMerchantCredentials>
{
public:
    void set_service_id(std::string value)
    {
        service_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_service_id() const noexcept
    {
        return service_id;
    }

    void set_merchant_display_name(std::string value)
    {
        merchant_display_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_display_name() const noexcept
    {
        return merchant_display_name;
    }

    void set_merchant_business_country(CountryAlpha2Value value)
    {
        merchant_business_country = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_merchant_business_country() const noexcept
    {
        return merchant_business_country;
    }

    void set_allowed_brands(std::vector<std::string> value)
    {
        allowed_brands = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_allowed_brands() const noexcept
    {
        return allowed_brands;
    }

    std::string service_id{};
    std::string merchant_display_name{};
    CountryAlpha2Value merchant_business_country{};
    std::vector<std::string> allowed_brands{};
};

class SamsungPayMerchantPaymentInformation :
    public serde::Serializer<SamsungPayMerchantPaymentInformation>
{
public:
    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_url(std::optional<std::string> value)
    {
        url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_url() const noexcept
    {
        return url;
    }

    void set_country_code(CountryAlpha2Value value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_country_code() const noexcept
    {
        return country_code;
    }

    std::string name{};
    std::optional<std::string> url{};
    CountryAlpha2Value country_code{};
};

class SamsungPaySessionTokenData : public serde::Serializer<SamsungPaySessionTokenData>
{
public:
    void set_data(SamsungPayCombinedMetadata value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const SamsungPayCombinedMetadata& get_data() const noexcept
    {
        return data;
    }

    SamsungPayCombinedMetadata data{};
};

class SamsungPayTokenData : public serde::Serializer<SamsungPayTokenData>
{
public:
    void set_three_ds_type(std::optional<std::string> value)
    {
        three_ds_type = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_three_ds_type() const noexcept
    {
        return three_ds_type;
    }

    void set_version(std::string value)
    {
        version = std::move(value);
    }

    [[nodiscard]] const std::string& get_version() const noexcept
    {
        return version;
    }

    void set_data(SecretValue value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_data() const noexcept
    {
        return data;
    }

    std::optional<std::string> three_ds_type{};
    std::string version{};
    SecretValue data{};
};

class SamsungPayWalletData : public serde::Serializer<SamsungPayWalletData>
{
public:
    void set_payment_credential(SamsungPayWalletCredentials value)
    {
        payment_credential = std::move(value);
    }

    [[nodiscard]] const SamsungPayWalletCredentials& get_payment_credential() const noexcept
    {
        return payment_credential;
    }

    SamsungPayWalletCredentials payment_credential{};
};

class SamsungPayAppWalletData : public serde::Serializer<SamsungPayAppWalletData>
{
public:
    void set_token_data(SamsungPayTokenData value)
    {
        token_data = std::move(value);
    }

    [[nodiscard]] const SamsungPayTokenData& get_token_data() const noexcept
    {
        return token_data;
    }

    void set_payment_card_brand(SamsungPayCardBrand value)
    {
        payment_card_brand = std::move(value);
    }

    [[nodiscard]] const SamsungPayCardBrand& get_payment_card_brand() const noexcept
    {
        return payment_card_brand;
    }

    void set_payment_currency_type(std::string value)
    {
        payment_currency_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_currency_type() const noexcept
    {
        return payment_currency_type;
    }

    void set_payment_last4_dpan(std::optional<std::string> value)
    {
        payment_last4_dpan = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payment_last4_dpan() const noexcept
    {
        return payment_last4_dpan;
    }

    void set_payment_last4_fpan(std::string value)
    {
        payment_last4_fpan = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_last4_fpan() const noexcept
    {
        return payment_last4_fpan;
    }

    void set_merchant_ref(std::optional<std::string> value)
    {
        merchant_ref = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_ref() const noexcept
    {
        return merchant_ref;
    }

    void set_method(std::optional<std::string> value)
    {
        method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_method() const noexcept
    {
        return method;
    }

    void set_recurring_payment(std::optional<bool> value)
    {
        recurring_payment = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_payment() const noexcept
    {
        return recurring_payment;
    }

    SamsungPayTokenData token_data{};
    SamsungPayCardBrand payment_card_brand{};
    std::string payment_currency_type{};
    std::optional<std::string> payment_last4_dpan{};
    std::string payment_last4_fpan{};
    std::optional<std::string> merchant_ref{};
    std::optional<std::string> method{};
    std::optional<bool> recurring_payment{};
};

class SamsungPaySessionTokenResponse : public serde::Serializer<SamsungPaySessionTokenResponse>
{
public:
    void set_version(std::string value)
    {
        version = std::move(value);
    }

    [[nodiscard]] const std::string& get_version() const noexcept
    {
        return version;
    }

    void set_service_id(std::string value)
    {
        service_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_service_id() const noexcept
    {
        return service_id;
    }

    void set_order_number(std::string value)
    {
        order_number = std::move(value);
    }

    [[nodiscard]] const std::string& get_order_number() const noexcept
    {
        return order_number;
    }

    void set_merchant_payment_information(SamsungPayMerchantPaymentInformation value)
    {
        merchant_payment_information = std::move(value);
    }

    [[nodiscard]] const SamsungPayMerchantPaymentInformation&
    get_merchant_payment_information() const noexcept
    {
        return merchant_payment_information;
    }

    void set_amount(SamsungPayAmountDetails value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const SamsungPayAmountDetails& get_amount() const noexcept
    {
        return amount;
    }

    void set_protocol(SamsungPayProtocolType value)
    {
        protocol = std::move(value);
    }

    [[nodiscard]] const SamsungPayProtocolType& get_protocol() const noexcept
    {
        return protocol;
    }

    void set_allowed_brands(std::vector<std::string> value)
    {
        allowed_brands = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_allowed_brands() const noexcept
    {
        return allowed_brands;
    }

    void set_billing_address_required(bool value)
    {
        billing_address_required = std::move(value);
    }

    [[nodiscard]] const bool& get_billing_address_required() const noexcept
    {
        return billing_address_required;
    }

    void set_shipping_address_required(bool value)
    {
        shipping_address_required = std::move(value);
    }

    [[nodiscard]] const bool& get_shipping_address_required() const noexcept
    {
        return shipping_address_required;
    }

    std::string version{};
    std::string service_id{};
    std::string order_number{};
    SamsungPayMerchantPaymentInformation merchant_payment_information{};
    SamsungPayAmountDetails amount{};
    SamsungPayProtocolType protocol{};
    std::vector<std::string> allowed_brands{};
    bool billing_address_required{};
    bool shipping_address_required{};
};

class SamsungPayWebWalletData : public serde::Serializer<SamsungPayWebWalletData>
{
public:
    void set_method(std::optional<std::string> value)
    {
        method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_method() const noexcept
    {
        return method;
    }

    void set_recurring_payment(std::optional<bool> value)
    {
        recurring_payment = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_recurring_payment() const noexcept
    {
        return recurring_payment;
    }

    void set_card_brand(SamsungPayCardBrand value)
    {
        card_brand = std::move(value);
    }

    [[nodiscard]] const SamsungPayCardBrand& get_card_brand() const noexcept
    {
        return card_brand;
    }

    void set_card_last_four_digits(std::string value)
    {
        card_last_four_digits = std::move(value);
    }

    [[nodiscard]] const std::string& get_card_last_four_digits() const noexcept
    {
        return card_last_four_digits;
    }

    void set_token_data(SamsungPayTokenData value)
    {
        token_data = std::move(value);
    }

    [[nodiscard]] const SamsungPayTokenData& get_token_data() const noexcept
    {
        return token_data;
    }

    std::optional<std::string> method{};
    std::optional<bool> recurring_payment{};
    SamsungPayCardBrand card_brand{};
    std::string card_last_four_digits{};
    SamsungPayTokenData token_data{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::SamsungPayWalletData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payment_credential",
            &app::model::payments::SamsungPayWalletData::get_payment_credential,
            &app::model::payments::SamsungPayWalletData::set_payment_credential,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_wallet_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPayAppWalletData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "3_d_s",
                &app::model::payments::SamsungPayAppWalletData::get_token_data,
                &app::model::payments::SamsungPayAppWalletData::set_token_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_card_brand",
                &app::model::payments::SamsungPayAppWalletData::get_payment_card_brand,
                &app::model::payments::SamsungPayAppWalletData::set_payment_card_brand,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_currency_type",
                &app::model::payments::SamsungPayAppWalletData::get_payment_currency_type,
                &app::model::payments::SamsungPayAppWalletData::set_payment_currency_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_last4_dpan",
                &app::model::payments::SamsungPayAppWalletData::get_payment_last4_dpan,
                &app::model::payments::SamsungPayAppWalletData::set_payment_last4_dpan,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_last4_fpan",
                &app::model::payments::SamsungPayAppWalletData::get_payment_last4_fpan,
                &app::model::payments::SamsungPayAppWalletData::set_payment_last4_fpan,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_ref",
                &app::model::payments::SamsungPayAppWalletData::get_merchant_ref,
                &app::model::payments::SamsungPayAppWalletData::set_merchant_ref,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "method",
                &app::model::payments::SamsungPayAppWalletData::get_method,
                &app::model::payments::SamsungPayAppWalletData::set_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment",
                &app::model::payments::SamsungPayAppWalletData::get_recurring_payment,
                &app::model::payments::SamsungPayAppWalletData::set_recurring_payment,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_app_wallet_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPayWebWalletData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "method",
                &app::model::payments::SamsungPayWebWalletData::get_method,
                &app::model::payments::SamsungPayWebWalletData::set_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "recurring_payment",
                &app::model::payments::SamsungPayWebWalletData::get_recurring_payment,
                &app::model::payments::SamsungPayWebWalletData::set_recurring_payment,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_brand",
                &app::model::payments::SamsungPayWebWalletData::get_card_brand,
                &app::model::payments::SamsungPayWebWalletData::set_card_brand,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_last4digits",
                &app::model::payments::SamsungPayWebWalletData::get_card_last_four_digits,
                &app::model::payments::SamsungPayWebWalletData::set_card_last_four_digits,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "3_d_s",
                &app::model::payments::SamsungPayWebWalletData::get_token_data,
                &app::model::payments::SamsungPayWebWalletData::set_token_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_web_wallet_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPayTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "type",
                &app::model::payments::SamsungPayTokenData::get_three_ds_type,
                &app::model::payments::SamsungPayTokenData::set_three_ds_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "version",
                &app::model::payments::SamsungPayTokenData::get_version,
                &app::model::payments::SamsungPayTokenData::set_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data",
                &app::model::payments::SamsungPayTokenData::get_data,
                &app::model::payments::SamsungPayTokenData::set_data,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPaySessionTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "samsung_pay",
            &app::model::payments::SamsungPaySessionTokenData::get_data,
            &app::model::payments::SamsungPaySessionTokenData::set_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_session_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPayMerchantCredentials>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "service_id",
                &app::model::payments::SamsungPayMerchantCredentials::get_service_id,
                &app::model::payments::SamsungPayMerchantCredentials::set_service_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_display_name",
                &app::model::payments::SamsungPayMerchantCredentials::get_merchant_display_name,
                &app::model::payments::SamsungPayMerchantCredentials::set_merchant_display_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_business_country",
                &app::model::payments::SamsungPayMerchantCredentials::get_merchant_business_country,
                &app::model::payments::SamsungPayMerchantCredentials::set_merchant_business_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allowed_brands",
                &app::model::payments::SamsungPayMerchantCredentials::get_allowed_brands,
                &app::model::payments::SamsungPayMerchantCredentials::set_allowed_brands,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_merchant_credentials";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPayApplicationCredentials>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_display_name",
                &app::model::payments::SamsungPayApplicationCredentials::get_merchant_display_name,
                &app::model::payments::SamsungPayApplicationCredentials::set_merchant_display_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_business_country",
                &app::model::payments::SamsungPayApplicationCredentials::
                    get_merchant_business_country,
                &app::model::payments::SamsungPayApplicationCredentials::
                    set_merchant_business_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allowed_brands",
                &app::model::payments::SamsungPayApplicationCredentials::get_allowed_brands,
                &app::model::payments::SamsungPayApplicationCredentials::set_allowed_brands,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_application_credentials";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPaySessionTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "version",
                &app::model::payments::SamsungPaySessionTokenResponse::get_version,
                &app::model::payments::SamsungPaySessionTokenResponse::set_version,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "service_id",
                &app::model::payments::SamsungPaySessionTokenResponse::get_service_id,
                &app::model::payments::SamsungPaySessionTokenResponse::set_service_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_number",
                &app::model::payments::SamsungPaySessionTokenResponse::get_order_number,
                &app::model::payments::SamsungPaySessionTokenResponse::set_order_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant",
                &app::model::payments::SamsungPaySessionTokenResponse::
                    get_merchant_payment_information,
                &app::model::payments::SamsungPaySessionTokenResponse::
                    set_merchant_payment_information,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "amount",
                &app::model::payments::SamsungPaySessionTokenResponse::get_amount,
                &app::model::payments::SamsungPaySessionTokenResponse::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "protocol",
                &app::model::payments::SamsungPaySessionTokenResponse::get_protocol,
                &app::model::payments::SamsungPaySessionTokenResponse::set_protocol,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "allowed_brands",
                &app::model::payments::SamsungPaySessionTokenResponse::get_allowed_brands,
                &app::model::payments::SamsungPaySessionTokenResponse::set_allowed_brands,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "billing_address_required",
                &app::model::payments::SamsungPaySessionTokenResponse::get_billing_address_required,
                &app::model::payments::SamsungPaySessionTokenResponse::set_billing_address_required,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_address_required",
                &app::model::payments::SamsungPaySessionTokenResponse::
                    get_shipping_address_required,
                &app::model::payments::SamsungPaySessionTokenResponse::
                    set_shipping_address_required,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_session_token_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPayMerchantPaymentInformation>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::model::payments::SamsungPayMerchantPaymentInformation::get_name,
                &app::model::payments::SamsungPayMerchantPaymentInformation::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "url",
                &app::model::payments::SamsungPayMerchantPaymentInformation::get_url,
                &app::model::payments::SamsungPayMerchantPaymentInformation::set_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &app::model::payments::SamsungPayMerchantPaymentInformation::get_country_code,
                &app::model::payments::SamsungPayMerchantPaymentInformation::set_country_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_merchant_payment_information";
    }
};

template<>
struct serde::Serializable<app::model::payments::SamsungPayAmountDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "option",
                &app::model::payments::SamsungPayAmountDetails::get_amount_format,
                &app::model::payments::SamsungPayAmountDetails::set_amount_format,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency_code",
                &app::model::payments::SamsungPayAmountDetails::get_currency_code,
                &app::model::payments::SamsungPayAmountDetails::set_currency_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total",
                &app::model::payments::SamsungPayAmountDetails::get_total_amount,
                &app::model::payments::SamsungPayAmountDetails::set_total_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "samsung_pay_amount_details";
    }
};
