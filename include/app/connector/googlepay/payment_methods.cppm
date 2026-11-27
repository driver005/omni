module;
export module app_connector:model_googlepay_payment_methods;

import std;
import core_serde;
import :model_shared_payment_methods;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payment_methods {

class GooglePayDecryptedInfo : public serde::Serializer<GooglePayDecryptedInfo>
{
public:
    void set_decrypted_data(GPayPredecryptDataValue value)
    {
        decrypted_data = std::move(value);
    }

    [[nodiscard]] const GPayPredecryptDataValue& get_decrypted_data() const noexcept
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

    GPayPredecryptDataValue decrypted_data{};
    PaymentMethodDataWalletInfo wallet_info{};
};

} // namespace app::model::payment_methods

template<>
struct serde::Serializable<app::model::payment_methods::GooglePayDecryptedInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "decrypted_data",
                &app::model::payment_methods::GooglePayDecryptedInfo::get_decrypted_data,
                &app::model::payment_methods::GooglePayDecryptedInfo::set_decrypted_data,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "wallet_info",
                &app::model::payment_methods::GooglePayDecryptedInfo::get_wallet_info,
                &app::model::payment_methods::GooglePayDecryptedInfo::set_wallet_info,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_decrypted_info";
    }
};
