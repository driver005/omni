module;
export module app_connector:model_ach_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class AchBankTransfer : public serde::Serializer<AchBankTransfer>
{
public:
    void set_bank_name(std::optional<std::string> value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_name() const noexcept
    {
        return bank_name;
    }

    void set_bank_country_code(std::optional<CountryAlpha2Value> value)
    {
        bank_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>& get_bank_country_code() const noexcept
    {
        return bank_country_code;
    }

    void set_bank_city(std::optional<std::string> value)
    {
        bank_city = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_city() const noexcept
    {
        return bank_city;
    }

    void set_bank_account_number(SecretValue value)
    {
        bank_account_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_bank_account_number() const noexcept
    {
        return bank_account_number;
    }

    void set_bank_routing_number(SecretValue value)
    {
        bank_routing_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_bank_routing_number() const noexcept
    {
        return bank_routing_number;
    }

    void set_account_holder_name(std::optional<SecretValue> value)
    {
        account_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept
    {
        return account_holder_name;
    }

    std::optional<std::string> bank_name{};
    std::optional<CountryAlpha2Value> bank_country_code{};
    std::optional<std::string> bank_city{};
    SecretValue bank_account_number{};
    SecretValue bank_routing_number{};
    std::optional<SecretValue> account_holder_name{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::AchBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_name",
                &app::model::payouts::AchBankTransfer::get_bank_name,
                &app::model::payouts::AchBankTransfer::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_country_code",
                &app::model::payouts::AchBankTransfer::get_bank_country_code,
                &app::model::payouts::AchBankTransfer::set_bank_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_city",
                &app::model::payouts::AchBankTransfer::get_bank_city,
                &app::model::payouts::AchBankTransfer::set_bank_city,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_number",
                &app::model::payouts::AchBankTransfer::get_bank_account_number,
                &app::model::payouts::AchBankTransfer::set_bank_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_routing_number",
                &app::model::payouts::AchBankTransfer::get_bank_routing_number,
                &app::model::payouts::AchBankTransfer::set_bank_routing_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payouts::AchBankTransfer::get_account_holder_name,
                &app::model::payouts::AchBankTransfer::set_account_holder_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "ach_bank_transfer";
    }
};
