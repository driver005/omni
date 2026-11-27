module;
export module app_connector:model_trustly_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class TrustlyBankTransfer : public serde::Serializer<TrustlyBankTransfer>
{
public:
    void set_iban(std::optional<SecretValue> value)
    {
        iban = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_iban() const noexcept
    {
        return iban;
    }

    void set_country_code(CountryAlpha2Value value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_country_code() const noexcept
    {
        return country_code;
    }

    void set_account_number(std::optional<SecretValue> value)
    {
        account_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_account_number() const noexcept
    {
        return account_number;
    }

    void set_bank_number(std::optional<SecretValue> value)
    {
        bank_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_bank_number() const noexcept
    {
        return bank_number;
    }

    std::optional<SecretValue> iban{};
    CountryAlpha2Value country_code{};
    std::optional<SecretValue> account_number{};
    std::optional<SecretValue> bank_number{};
};

class TrustlyBankTransferData : public serde::Serializer<TrustlyBankTransferData>
{
public:
    void set_iban(std::optional<SecretValue> value)
    {
        iban = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_iban() const noexcept
    {
        return iban;
    }

    void set_bank_country_code(CountryAlpha2Value value)
    {
        bank_country_code = std::move(value);
    }

    [[nodiscard]] const CountryAlpha2Value& get_bank_country_code() const noexcept
    {
        return bank_country_code;
    }

    void set_bank_account_number(std::optional<SecretValue> value)
    {
        bank_account_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_bank_account_number() const noexcept
    {
        return bank_account_number;
    }

    void set_bank_number(std::optional<SecretValue> value)
    {
        bank_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_bank_number() const noexcept
    {
        return bank_number;
    }

    std::optional<SecretValue> iban{};
    CountryAlpha2Value bank_country_code{};
    std::optional<SecretValue> bank_account_number{};
    std::optional<SecretValue> bank_number{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::TrustlyBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "iban",
                &app::model::payouts::TrustlyBankTransfer::get_iban,
                &app::model::payouts::TrustlyBankTransfer::set_iban,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &app::model::payouts::TrustlyBankTransfer::get_country_code,
                &app::model::payouts::TrustlyBankTransfer::set_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_number",
                &app::model::payouts::TrustlyBankTransfer::get_account_number,
                &app::model::payouts::TrustlyBankTransfer::set_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_number",
                &app::model::payouts::TrustlyBankTransfer::get_bank_number,
                &app::model::payouts::TrustlyBankTransfer::set_bank_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "trustly_bank_transfer";
    }
};

template<>
struct serde::Serializable<app::model::payouts::TrustlyBankTransferData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "iban",
                &app::model::payouts::TrustlyBankTransferData::get_iban,
                &app::model::payouts::TrustlyBankTransferData::set_iban,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_country_code",
                &app::model::payouts::TrustlyBankTransferData::get_bank_country_code,
                &app::model::payouts::TrustlyBankTransferData::set_bank_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_number",
                &app::model::payouts::TrustlyBankTransferData::get_bank_account_number,
                &app::model::payouts::TrustlyBankTransferData::set_bank_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_number",
                &app::model::payouts::TrustlyBankTransferData::get_bank_number,
                &app::model::payouts::TrustlyBankTransferData::set_bank_number,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "trustly_bank_transfer_data";
    }
};
