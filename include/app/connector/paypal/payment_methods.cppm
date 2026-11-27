module;
export module app_connector:model_paypal_payment_methods;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payment_methods {

class PaypalWalletInfo : public serde::Serializer<PaypalWalletInfo>
{
public:
    void set_payer_id(std::optional<std::string> value)
    {
        payer_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_payer_id() const noexcept
    {
        return payer_id;
    }

    std::optional<std::string> payer_id{};
};

} // namespace app::model::payment_methods

template<>
struct serde::Serializable<app::model::payment_methods::PaypalWalletInfo>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payer_id",
            &app::model::payment_methods::PaypalWalletInfo::get_payer_id,
            &app::model::payment_methods::PaypalWalletInfo::set_payer_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "paypal_wallet_info";
    }
};
