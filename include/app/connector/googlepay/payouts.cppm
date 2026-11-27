module;
export module app_connector:model_googlepay_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class GooglePayDecrypt : public serde::Serializer<GooglePayDecrypt>
{
public:
    void set_application_primary_account_number(CardNumberValue value)
    {
        application_primary_account_number = std::move(value);
    }

    [[nodiscard]] const CardNumberValue& get_application_primary_account_number() const noexcept
    {
        return application_primary_account_number;
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

    CardNumberValue application_primary_account_number{};
    SecretValue expiry_month{};
    SecretValue expiry_year{};
    std::optional<SecretValue> card_holder_name{};
    std::optional<CardNetworkValue> card_network{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::GooglePayDecrypt>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "application_primary_account_number",
                &app::model::payouts::GooglePayDecrypt::get_application_primary_account_number,
                &app::model::payouts::GooglePayDecrypt::set_application_primary_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_month",
                &app::model::payouts::GooglePayDecrypt::get_expiry_month,
                &app::model::payouts::GooglePayDecrypt::set_expiry_month,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expiry_year",
                &app::model::payouts::GooglePayDecrypt::get_expiry_year,
                &app::model::payouts::GooglePayDecrypt::set_expiry_year,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_holder_name",
                &app::model::payouts::GooglePayDecrypt::get_card_holder_name,
                &app::model::payouts::GooglePayDecrypt::set_card_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "card_network",
                &app::model::payouts::GooglePayDecrypt::get_card_network,
                &app::model::payouts::GooglePayDecrypt::set_card_network,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "google_pay_decrypt";
    }
};
