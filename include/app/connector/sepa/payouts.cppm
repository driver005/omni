module;
export module app_connector:model_sepa_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class SepaBankTransfer : public serde::Serializer<SepaBankTransfer>
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

    void set_iban(SecretValue value)
    {
        iban = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_iban() const noexcept
    {
        return iban;
    }

    void set_bic(std::optional<SecretValue> value)
    {
        bic = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_bic() const noexcept
    {
        return bic;
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
    SecretValue iban{};
    std::optional<SecretValue> bic{};
    std::optional<SecretValue> account_holder_name{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::SepaBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_name",
                &app::model::payouts::SepaBankTransfer::get_bank_name,
                &app::model::payouts::SepaBankTransfer::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_country_code",
                &app::model::payouts::SepaBankTransfer::get_bank_country_code,
                &app::model::payouts::SepaBankTransfer::set_bank_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_city",
                &app::model::payouts::SepaBankTransfer::get_bank_city,
                &app::model::payouts::SepaBankTransfer::set_bank_city,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "iban",
                &app::model::payouts::SepaBankTransfer::get_iban,
                &app::model::payouts::SepaBankTransfer::set_iban,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bic",
                &app::model::payouts::SepaBankTransfer::get_bic,
                &app::model::payouts::SepaBankTransfer::set_bic,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payouts::SepaBankTransfer::get_account_holder_name,
                &app::model::payouts::SepaBankTransfer::set_account_holder_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sepa_bank_transfer";
    }
};
