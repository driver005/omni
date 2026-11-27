module;
export module app_connector:model_paypal_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class PayPalWalletData : public serde::Serializer<PayPalWalletData>
{
public:
    void set_token(std::string value)
    {
        token = std::move(value);
    }

    [[nodiscard]] const std::string& get_token() const noexcept
    {
        return token;
    }

    std::string token{};
};

enum class PaypalCaptureMethod
{
    Capture,
    Authorize,
};

enum class PaypalFlow
{
    Checkout,
};

class PaypalSdkMetaData : public serde::Serializer<PaypalSdkMetaData>
{
public:
    void set_client_id(std::string value)
    {
        client_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_client_id() const noexcept
    {
        return client_id;
    }

    std::string client_id{};
};

class PaypalRedirection : public serde::Serializer<PaypalRedirection>
{
public:
    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_payer_id(std::optional<SecretValue> value)
    {
        payer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_payer_id() const noexcept
    {
        return payer_id;
    }

    std::optional<EmailValue> email{};
    std::optional<SecretValue> payer_id{};
};

class PaypalSdkSessionTokenData : public serde::Serializer<PaypalSdkSessionTokenData>
{
public:
    void set_data(PaypalSdkMetaData value)
    {
        data = std::move(value);
    }

    [[nodiscard]] const PaypalSdkMetaData& get_data() const noexcept
    {
        return data;
    }

    PaypalSdkMetaData data{};
};

class PaypalTransactionInfo : public serde::Serializer<PaypalTransactionInfo>
{
public:
    void set_flow(PaypalFlow value)
    {
        flow = std::move(value);
    }

    [[nodiscard]] const PaypalFlow& get_flow() const noexcept
    {
        return flow;
    }

    void set_currency_code(CurrencyValue value)
    {
        currency_code = std::move(value);
    }

    [[nodiscard]] const CurrencyValue& get_currency_code() const noexcept
    {
        return currency_code;
    }

    void set_total_price(StringMajorUnitValue value)
    {
        total_price = std::move(value);
    }

    [[nodiscard]] const StringMajorUnitValue& get_total_price() const noexcept
    {
        return total_price;
    }

    PaypalFlow flow{};
    CurrencyValue currency_code{};
    StringMajorUnitValue total_price{};
};

class PaypalSessionTokenResponse : public serde::Serializer<PaypalSessionTokenResponse>
{
public:
    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_session_token(std::string value)
    {
        session_token = std::move(value);
    }

    [[nodiscard]] const std::string& get_session_token() const noexcept
    {
        return session_token;
    }

    void set_sdk_next_action(SdkNextAction value)
    {
        sdk_next_action = std::move(value);
    }

    [[nodiscard]] const SdkNextAction& get_sdk_next_action() const noexcept
    {
        return sdk_next_action;
    }

    void set_client_token(std::optional<std::string> value)
    {
        client_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_client_token() const noexcept
    {
        return client_token;
    }

    void set_transaction_info(std::optional<PaypalTransactionInfo> value)
    {
        transaction_info = std::move(value);
    }

    [[nodiscard]] const std::optional<PaypalTransactionInfo>& get_transaction_info() const noexcept
    {
        return transaction_info;
    }

    void set_data_user_id_token(std::optional<std::string> value)
    {
        data_user_id_token = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_data_user_id_token() const noexcept
    {
        return data_user_id_token;
    }

    void set_currency(std::optional<CurrencyValue> value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::optional<CurrencyValue>& get_currency() const noexcept
    {
        return currency;
    }

    void set_intent(std::optional<PaypalCaptureMethod> value)
    {
        intent = std::move(value);
    }

    [[nodiscard]] const std::optional<PaypalCaptureMethod>& get_intent() const noexcept
    {
        return intent;
    }

    std::string connector{};
    std::string session_token{};
    SdkNextAction sdk_next_action{};
    std::optional<std::string> client_token{};
    std::optional<PaypalTransactionInfo> transaction_info{};
    std::optional<std::string> data_user_id_token{};
    std::optional<CurrencyValue> currency{};
    std::optional<PaypalCaptureMethod> intent{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::PaypalRedirection>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "email",
                &app::model::payments::PaypalRedirection::get_email,
                &app::model::payments::PaypalRedirection::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payer_id",
                &app::model::payments::PaypalRedirection::get_payer_id,
                &app::model::payments::PaypalRedirection::set_payer_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "paypal_redirection";
    }
};

template<>
struct serde::Serializable<app::model::payments::PayPalWalletData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "token",
            &app::model::payments::PayPalWalletData::get_token,
            &app::model::payments::PayPalWalletData::set_token,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "pay_pal_wallet_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaypalSdkMetaData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "client_id",
            &app::model::payments::PaypalSdkMetaData::get_client_id,
            &app::model::payments::PaypalSdkMetaData::set_client_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "paypal_sdk_meta_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaypalSdkSessionTokenData>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "paypal_sdk",
            &app::model::payments::PaypalSdkSessionTokenData::get_data,
            &app::model::payments::PaypalSdkSessionTokenData::set_data,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "paypal_sdk_session_token_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaypalTransactionInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "flow",
                &app::model::payments::PaypalTransactionInfo::get_flow,
                &app::model::payments::PaypalTransactionInfo::set_flow,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency_code",
                &app::model::payments::PaypalTransactionInfo::get_currency_code,
                &app::model::payments::PaypalTransactionInfo::set_currency_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "total_price",
                &app::model::payments::PaypalTransactionInfo::get_total_price,
                &app::model::payments::PaypalTransactionInfo::set_total_price,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "paypal_transaction_info";
    }
};

template<>
struct serde::Serializable<app::model::payments::PaypalSessionTokenResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::model::payments::PaypalSessionTokenResponse::get_connector,
                &app::model::payments::PaypalSessionTokenResponse::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "session_token",
                &app::model::payments::PaypalSessionTokenResponse::get_session_token,
                &app::model::payments::PaypalSessionTokenResponse::set_session_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sdk_next_action",
                &app::model::payments::PaypalSessionTokenResponse::get_sdk_next_action,
                &app::model::payments::PaypalSessionTokenResponse::set_sdk_next_action,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "client_token",
                &app::model::payments::PaypalSessionTokenResponse::get_client_token,
                &app::model::payments::PaypalSessionTokenResponse::set_client_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "transaction_info",
                &app::model::payments::PaypalSessionTokenResponse::get_transaction_info,
                &app::model::payments::PaypalSessionTokenResponse::set_transaction_info,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "data_user_id_token",
                &app::model::payments::PaypalSessionTokenResponse::get_data_user_id_token,
                &app::model::payments::PaypalSessionTokenResponse::set_data_user_id_token,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::model::payments::PaypalSessionTokenResponse::get_currency,
                &app::model::payments::PaypalSessionTokenResponse::set_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "intent",
                &app::model::payments::PaypalSessionTokenResponse::get_intent,
                &app::model::payments::PaypalSessionTokenResponse::set_intent,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "paypal_session_token_response";
    }
};
