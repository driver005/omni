module;
export module app_connector:model_applepay_payment_methods;

import std;
import core_serde;
import :model_shared_payment_methods;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payment_methods {

class ApplePayPredecryptDataValue : public serde::Serializer<ApplePayPredecryptDataValue>
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ApplePayDecryptedInfo : public serde::Serializer<ApplePayDecryptedInfo>
{
public:
    void set_decrypted_data(ApplePayPredecryptDataValue value)
    {
        decrypted_data = std::move(value);
    }

    [[nodiscard]] const ApplePayPredecryptDataValue& get_decrypted_data() const noexcept
    {
        return decrypted_data;
    }

    void set_wallet_info(PaymentMethodDataWalletInfo value)
    {
        wallet_info = std::move(value);
    }

    [[nodiscard]] const PaymentMethodDataWalletInfo& get_wallet_info() const noexcept
    {
        return wallet_info;
    }

    ApplePayPredecryptDataValue decrypted_data{};
    PaymentMethodDataWalletInfo wallet_info{};
};

} // namespace app::model::payment_methods

template<>
struct serde::Serializable<app::model::payment_methods::ApplePayDecryptedInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "decrypted_data",
                &app::model::payment_methods::ApplePayDecryptedInfo::get_decrypted_data,
                &app::model::payment_methods::ApplePayDecryptedInfo::set_decrypted_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "wallet_info",
                &app::model::payment_methods::ApplePayDecryptedInfo::get_wallet_info,
                &app::model::payment_methods::ApplePayDecryptedInfo::set_wallet_info,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_decrypted_info";
    }
};
