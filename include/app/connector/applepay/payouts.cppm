module;
export module app_connector:model_applepay_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class ApplePayDecrypt : public serde::Serializer<ApplePayDecrypt>
{
public:
    void set_dpan(CardNumberValue value)
    {
        dpan = std::move(value);
    }

    [[nodiscard]] const CardNumberValue& get_dpan() const noexcept
    {
        return dpan;
    }

    void set_expiry_month(SecretValue value)
    {
        expiry_month = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_expiry_month() const noexcept
    {
        return expiry_month;
    }

    void set_expiry_year(SecretValue value)
    {
        expiry_year = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_expiry_year() const noexcept
    {
        return expiry_year;
    }

    void set_card_holder_name(std::optional<SecretValue> value)
    {
        card_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_card_holder_name() const noexcept
    {
        return card_holder_name;
    }

    void set_card_network(std::optional<CardNetworkValue> value)
    {
        card_network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_card_network() const noexcept
    {
        return card_network;
    }

    CardNumberValue dpan{};
    SecretValue expiry_month{};
    SecretValue expiry_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<CardNetworkValue> card_network{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::ApplePayDecrypt>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "dpan",
                &app::model::payouts::ApplePayDecrypt::get_dpan,
                &app::model::payouts::ApplePayDecrypt::set_dpan,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_month",
                &app::model::payouts::ApplePayDecrypt::get_expiry_month,
                &app::model::payouts::ApplePayDecrypt::set_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_year",
                &app::model::payouts::ApplePayDecrypt::get_expiry_year,
                &app::model::payouts::ApplePayDecrypt::set_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payouts::ApplePayDecrypt::get_card_holder_name,
                &app::model::payouts::ApplePayDecrypt::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payouts::ApplePayDecrypt::get_card_network,
                &app::model::payouts::ApplePayDecrypt::set_card_network,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "apple_pay_decrypt";
    }
};
