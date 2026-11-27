module;
export module app_connector:model_ted_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class TedBankTransfer : public serde::Serializer<TedBankTransfer>
{
public:
    void set_bank_name(std::optional<BankNamesValue> value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const std::optional<BankNamesValue>& get_bank_name() const noexcept
    {
        return bank_name;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_ispb(std::optional<std::string> value)
    {
        ispb = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_ispb() const noexcept
    {
        return ispb;
    }

    void set_bank_branch(std::optional<std::string> value)
    {
        bank_branch = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_branch() const noexcept
    {
        return bank_branch;
    }

    void set_bank_account_number(SecretValue value)
    {
        bank_account_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_bank_account_number() const noexcept
    {
        return bank_account_number;
    }

    void set_bank_account_type(std::optional<BankTypeValue> value)
    {
        bank_account_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BankTypeValue>& get_bank_account_type() const noexcept
    {
        return bank_account_type;
    }

    void set_tax_id(std::optional<SecretValue> value)
    {
        tax_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_tax_id() const noexcept
    {
        return tax_id;
    }

    void set_account_holder_name(std::optional<SecretValue> value)
    {
        account_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept
    {
        return account_holder_name;
    }

    std::optional<BankNamesValue> bank_name{};
    std::optional<std::string> bank_code{};
    std::optional<std::string> ispb{};
    std::optional<std::string> bank_branch{};
    SecretValue bank_account_number{};
    std::optional<BankTypeValue> bank_account_type{};
    std::optional<SecretValue> tax_id{};
    std::optional<SecretValue> account_holder_name{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::TedBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_name",
                &app::model::payouts::TedBankTransfer::get_bank_name,
                &app::model::payouts::TedBankTransfer::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payouts::TedBankTransfer::get_bank_code,
                &app::model::payouts::TedBankTransfer::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ispb",
                &app::model::payouts::TedBankTransfer::get_ispb,
                &app::model::payouts::TedBankTransfer::set_ispb,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_branch",
                &app::model::payouts::TedBankTransfer::get_bank_branch,
                &app::model::payouts::TedBankTransfer::set_bank_branch,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_number",
                &app::model::payouts::TedBankTransfer::get_bank_account_number,
                &app::model::payouts::TedBankTransfer::set_bank_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_type",
                &app::model::payouts::TedBankTransfer::get_bank_account_type,
                &app::model::payouts::TedBankTransfer::set_bank_account_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_id",
                &app::model::payouts::TedBankTransfer::get_tax_id,
                &app::model::payouts::TedBankTransfer::set_tax_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payouts::TedBankTransfer::get_account_holder_name,
                &app::model::payouts::TedBankTransfer::set_account_holder_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "ted_bank_transfer";
    }
};
