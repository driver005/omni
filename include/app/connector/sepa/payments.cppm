module;
export module app_connector:model_sepa_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class SepaBankTransferInstructions : public serde::Serializer<SepaBankTransferInstructions>
{
public:
    void set_account_holder_name(SecretValue value)
    {
        account_holder_name = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_account_holder_name() const noexcept
    {
        return account_holder_name;
    }

    void set_bic(SecretValue value)
    {
        bic = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_bic() const noexcept
    {
        return bic;
    }

    void set_country(std::string value)
    {
        country = std::move(value);
    }

    [[nodiscard]] const std::string& get_country() const noexcept
    {
        return country;
    }

    void set_iban(SecretValue value)
    {
        iban = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_iban() const noexcept
    {
        return iban;
    }

    void set_reference(SecretValue value)
    {
        reference = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_reference() const noexcept
    {
        return reference;
    }

    SecretValue account_holder_name{};
    SecretValue bic{};
    std::string country{};
    SecretValue iban{};
    SecretValue reference{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::SepaBankTransferInstructions>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payments::SepaBankTransferInstructions::get_account_holder_name,
                &app::model::payments::SepaBankTransferInstructions::set_account_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bic",
                &app::model::payments::SepaBankTransferInstructions::get_bic,
                &app::model::payments::SepaBankTransferInstructions::set_bic,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country",
                &app::model::payments::SepaBankTransferInstructions::get_country,
                &app::model::payments::SepaBankTransferInstructions::set_country,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "iban",
                &app::model::payments::SepaBankTransferInstructions::get_iban,
                &app::model::payments::SepaBankTransferInstructions::set_iban,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "reference",
                &app::model::payments::SepaBankTransferInstructions::get_reference,
                &app::model::payments::SepaBankTransferInstructions::set_reference,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "sepa_bank_transfer_instructions";
    }
};
