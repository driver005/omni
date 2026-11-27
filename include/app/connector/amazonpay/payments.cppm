module;
export module app_connector:model_amazonpay_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class AmazonPayMerchantCredentials : public serde::Serializer<AmazonPayMerchantCredentials>
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_store_id(std::string value)
    {
        store_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_store_id() const noexcept
    {
        return store_id;
    }

    std::string merchant_id{};
    std::string store_id{};
};

enum class AmazonPayPaymentIntent
{
    Confirm,
    Authorize,
    AuthorizeWithCapture,
};

class AmazonPayRedirectData : public serde::Serializer<AmazonPayRedirectData>
{
public:
};

class AmazonPayShippingMethod : public serde::Serializer<AmazonPayShippingMethod>
{
public:
    void set_shipping_method_name(std::string value)
    {
        shipping_method_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_shipping_method_name() const noexcept
    {
        return shipping_method_name;
    }

    void set_shipping_method_code(std::string value)
    {
        shipping_method_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_shipping_method_code() const noexcept
    {
        return shipping_method_code;
    }

    std::string shipping_method_name{};
    std::string shipping_method_code{};
};

class AmazonPayWalletData : public serde::Serializer<AmazonPayWalletData>
{
public:
    void set_checkout_session_id(std::string value)
    {
        checkout_session_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_checkout_session_id() const noexcept
    {
        return checkout_session_id;
    }

    std::string checkout_session_id{};
};

class AmazonPayDeliveryPrice : public serde::Serializer<AmazonPayDeliveryPrice>
{
public:
    void set_amount(MinorUnitValue value)
    {
        amount = std::move(value);
    }

    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept
    {
        return amount;
    }

    void set_display_amount(StringMajorUnitValue value)
    {
        display_amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_display_amount() const noexcept
    {
        return display_amount;
    }

    void set_currency_code(CurrencyValue value)
    {
        currency_code = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency_code() const noexcept
    {
        return currency_code;
    }

    MinorUnitValue amount{};
    StringMajorUnitValue display_amount{};
    CurrencyValue currency_code{};
};

class AmazonPaySessionTokenData : public serde::Serializer<AmazonPaySessionTokenData>
{
public:
    void set_data(AmazonPayMerchantCredentials value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const AmazonPayMerchantCredentials& get_data() const noexcept
    {
        return data;
    }

    AmazonPayMerchantCredentials data{};
};

class AmazonPayDeliveryOptions : public serde::Serializer<AmazonPayDeliveryOptions>
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_price(AmazonPayDeliveryPrice value)
    {
        price = std::move(value);
    }

    [[nodiscard]] const AmazonPayDeliveryPrice& get_price() const noexcept
    {
        return price;
    }

    void set_shipping_method(AmazonPayShippingMethod value)
    {
        shipping_method = std::move(value);
    }

    [[nodiscard]] const AmazonPayShippingMethod& get_shipping_method() const noexcept
    {
        return shipping_method;
    }

    void set_is_default(bool value)
    {
        is_default = std::move(value);
    }

    [[nodiscard]] const bool& get_is_default() const noexcept
    {
        return is_default;
    }

    std::string id{};
    AmazonPayDeliveryPrice price{};
    AmazonPayShippingMethod shipping_method{};
    bool is_default{};
};

class AmazonPaySessionTokenResponse : public serde::Serializer<AmazonPaySessionTokenResponse>
{
public:
    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_ledger_currency(CurrencyValue value)
    {
        ledger_currency = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_ledger_currency() const noexcept
    {
        return ledger_currency;
    }

    void set_store_id(std::string value)
    {
        store_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_store_id() const noexcept
    {
        return store_id;
    }

    void set_payment_intent(AmazonPayPaymentIntent value)
    {
        payment_intent = std::move(value);
    }

    [[nodiscard]] const AmazonPayPaymentIntent& get_payment_intent() const noexcept
    {
        return payment_intent;
    }

    void set_total_shipping_amount(StringMajorUnitValue value)
    {
        total_shipping_amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_total_shipping_amount() const noexcept
    {
        return total_shipping_amount;
    }

    void set_total_tax_amount(StringMajorUnitValue value)
    {
        total_tax_amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_total_tax_amount() const noexcept
    {
        return total_tax_amount;
    }

    void set_total_base_amount(StringMajorUnitValue value)
    {
        total_base_amount = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_total_base_amount() const noexcept
    {
        return total_base_amount;
    }

    void set_delivery_options(std::vector<AmazonPayDeliveryOptions> value)
    {
        delivery_options = std::move(value);
    }

    [[nodiscard]] const std::vector<AmazonPayDeliveryOptions>& get_delivery_options() const noexcept
    {
        return delivery_options;
    }

    std::string merchant_id{};
    CurrencyValue ledger_currency{};
    std::string store_id{};
    AmazonPayPaymentIntent payment_intent{};
    StringMajorUnitValue total_shipping_amount{};
    StringMajorUnitValue total_tax_amount{};
    StringMajorUnitValue total_base_amount{};
    std::vector<AmazonPayDeliveryOptions> delivery_options{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::AmazonPaySessionTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "amazon_pay",
            &app::model::payments::AmazonPaySessionTokenData::get_data,
            &app::model::payments::AmazonPaySessionTokenData::set_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "amazon_pay_session_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmazonPayMerchantCredentials>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::AmazonPayMerchantCredentials::get_merchant_id,
                &app::model::payments::AmazonPayMerchantCredentials::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "store_id",
                &app::model::payments::AmazonPayMerchantCredentials::get_store_id,
                &app::model::payments::AmazonPayMerchantCredentials::set_store_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amazon_pay_merchant_credentials";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmazonPayRedirectData>
{
    static constexpr auto fields()
    {
        return std::tuple{};
    }

    static constexpr std::string_view table_name()
    {
        return "amazon_pay_redirect_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmazonPayWalletData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "checkout_session_id",
            &app::model::payments::AmazonPayWalletData::get_checkout_session_id,
            &app::model::payments::AmazonPayWalletData::set_checkout_session_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "amazon_pay_wallet_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmazonPaySessionTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "merchant_id",
                &app::model::payments::AmazonPaySessionTokenResponse::get_merchant_id,
                &app::model::payments::AmazonPaySessionTokenResponse::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ledger_currency",
                &app::model::payments::AmazonPaySessionTokenResponse::get_ledger_currency,
                &app::model::payments::AmazonPaySessionTokenResponse::set_ledger_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "store_id",
                &app::model::payments::AmazonPaySessionTokenResponse::get_store_id,
                &app::model::payments::AmazonPaySessionTokenResponse::set_store_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_intent",
                &app::model::payments::AmazonPaySessionTokenResponse::get_payment_intent,
                &app::model::payments::AmazonPaySessionTokenResponse::set_payment_intent,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_shipping_amount",
                &app::model::payments::AmazonPaySessionTokenResponse::get_total_shipping_amount,
                &app::model::payments::AmazonPaySessionTokenResponse::set_total_shipping_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_tax_amount",
                &app::model::payments::AmazonPaySessionTokenResponse::get_total_tax_amount,
                &app::model::payments::AmazonPaySessionTokenResponse::set_total_tax_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_base_amount",
                &app::model::payments::AmazonPaySessionTokenResponse::get_total_base_amount,
                &app::model::payments::AmazonPaySessionTokenResponse::set_total_base_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "delivery_options",
                &app::model::payments::AmazonPaySessionTokenResponse::get_delivery_options,
                &app::model::payments::AmazonPaySessionTokenResponse::set_delivery_options,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amazon_pay_session_token_response";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmazonPayDeliveryOptions>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::model::payments::AmazonPayDeliveryOptions::get_id,
                &app::model::payments::AmazonPayDeliveryOptions::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "price",
                &app::model::payments::AmazonPayDeliveryOptions::get_price,
                &app::model::payments::AmazonPayDeliveryOptions::set_price,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_method",
                &app::model::payments::AmazonPayDeliveryOptions::get_shipping_method,
                &app::model::payments::AmazonPayDeliveryOptions::set_shipping_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_default",
                &app::model::payments::AmazonPayDeliveryOptions::get_is_default,
                &app::model::payments::AmazonPayDeliveryOptions::set_is_default,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amazon_pay_delivery_options";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmazonPayDeliveryPrice>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::model::payments::AmazonPayDeliveryPrice::get_amount,
                &app::model::payments::AmazonPayDeliveryPrice::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_amount",
                &app::model::payments::AmazonPayDeliveryPrice::get_display_amount,
                &app::model::payments::AmazonPayDeliveryPrice::set_display_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency_code",
                &app::model::payments::AmazonPayDeliveryPrice::get_currency_code,
                &app::model::payments::AmazonPayDeliveryPrice::set_currency_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amazon_pay_delivery_price";
    }
};

template<>
struct serde::Serializable<app::model::payments::AmazonPayShippingMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "shipping_method_name",
                &app::model::payments::AmazonPayShippingMethod::get_shipping_method_name,
                &app::model::payments::AmazonPayShippingMethod::set_shipping_method_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipping_method_code",
                &app::model::payments::AmazonPayShippingMethod::get_shipping_method_code,
                &app::model::payments::AmazonPayShippingMethod::set_shipping_method_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "amazon_pay_shipping_method";
    }
};
