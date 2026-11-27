module;
export module app_connector:model_pix_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class PixAccountBankTransfer : public serde::Serializer<PixAccountBankTransfer>
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

    void set_tax_id(std::optional<SecretValue> value)
    {
        tax_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_tax_id() const noexcept
    {
        return tax_id;
    }

    void set_ispb(std::optional<std::string> value)
    {
        ispb = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_ispb() const noexcept
    {
        return ispb;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_bank_account_type(std::optional<BankTypeValue> value)
    {
        bank_account_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BankTypeValue>& get_bank_account_type() const noexcept
    {
        return bank_account_type;
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
    std::optional<std::string> bank_branch{};
    SecretValue bank_account_number{};
    std::optional<SecretValue> tax_id{};
    std::optional<std::string> ispb{};
    std::optional<std::string> bank_code{};
    std::optional<BankTypeValue> bank_account_type{};
    std::optional<SecretValue> account_holder_name{};
};

class PixBankTransfer : public serde::Serializer<PixBankTransfer>
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

    void set_bank_branch(std::optional<std::string> value)
    {
        bank_branch = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_branch() const noexcept
    {
        return bank_branch;
    }

    void set_bank_account_number(std::optional<SecretValue> value)
    {
        bank_account_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_bank_account_number() const noexcept
    {
        return bank_account_number;
    }

    void set_pix_key(std::optional<SecretValue> value)
    {
        pix_key = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_pix_key() const noexcept
    {
        return pix_key;
    }

    void set_tax_id(std::optional<SecretValue> value)
    {
        tax_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_tax_id() const noexcept
    {
        return tax_id;
    }

    void set_emv(std::optional<SecretValue> value)
    {
        emv = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_emv() const noexcept
    {
        return emv;
    }

    void set_ispb(std::optional<std::string> value)
    {
        ispb = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_ispb() const noexcept
    {
        return ispb;
    }

    void set_bank_code(std::optional<std::string> value)
    {
        bank_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_bank_code() const noexcept
    {
        return bank_code;
    }

    void set_bank_account_type(std::optional<BankTypeValue> value)
    {
        bank_account_type = std::move(value);
    }

    [[nodiscard]] const std::optional<BankTypeValue>& get_bank_account_type() const noexcept
    {
        return bank_account_type;
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
    std::optional<std::string> bank_branch{};
    std::optional<SecretValue> bank_account_number{};
    std::optional<SecretValue> pix_key{};
    std::optional<SecretValue> tax_id{};
    std::optional<SecretValue> emv{};
    std::optional<std::string> ispb{};
    std::optional<std::string> bank_code{};
    std::optional<BankTypeValue> bank_account_type{};
    std::optional<SecretValue> account_holder_name{};
};

class PixEmvBankTransfer : public serde::Serializer<PixEmvBankTransfer>
{
public:
    void set_emv(SecretValue value)
    {
        emv = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_emv() const noexcept
    {
        return emv;
    }

    SecretValue emv{};
};

class PixKeyBankTransfer : public serde::Serializer<PixKeyBankTransfer>
{
public:
    void set_pix_key(SecretValue value)
    {
        pix_key = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_pix_key() const noexcept
    {
        return pix_key;
    }

    SecretValue pix_key{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::PixBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_name",
                &app::model::payouts::PixBankTransfer::get_bank_name,
                &app::model::payouts::PixBankTransfer::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_branch",
                &app::model::payouts::PixBankTransfer::get_bank_branch,
                &app::model::payouts::PixBankTransfer::set_bank_branch,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_number",
                &app::model::payouts::PixBankTransfer::get_bank_account_number,
                &app::model::payouts::PixBankTransfer::set_bank_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "pix_key",
                &app::model::payouts::PixBankTransfer::get_pix_key,
                &app::model::payouts::PixBankTransfer::set_pix_key,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_id",
                &app::model::payouts::PixBankTransfer::get_tax_id,
                &app::model::payouts::PixBankTransfer::set_tax_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "emv",
                &app::model::payouts::PixBankTransfer::get_emv,
                &app::model::payouts::PixBankTransfer::set_emv,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ispb",
                &app::model::payouts::PixBankTransfer::get_ispb,
                &app::model::payouts::PixBankTransfer::set_ispb,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payouts::PixBankTransfer::get_bank_code,
                &app::model::payouts::PixBankTransfer::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_type",
                &app::model::payouts::PixBankTransfer::get_bank_account_type,
                &app::model::payouts::PixBankTransfer::set_bank_account_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payouts::PixBankTransfer::get_account_holder_name,
                &app::model::payouts::PixBankTransfer::set_account_holder_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pix_bank_transfer";
    }
};

template<>
struct serde::Serializable<app::model::payouts::PixAccountBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_name",
                &app::model::payouts::PixAccountBankTransfer::get_bank_name,
                &app::model::payouts::PixAccountBankTransfer::set_bank_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_branch",
                &app::model::payouts::PixAccountBankTransfer::get_bank_branch,
                &app::model::payouts::PixAccountBankTransfer::set_bank_branch,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_number",
                &app::model::payouts::PixAccountBankTransfer::get_bank_account_number,
                &app::model::payouts::PixAccountBankTransfer::set_bank_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_id",
                &app::model::payouts::PixAccountBankTransfer::get_tax_id,
                &app::model::payouts::PixAccountBankTransfer::set_tax_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "ispb",
                &app::model::payouts::PixAccountBankTransfer::get_ispb,
                &app::model::payouts::PixAccountBankTransfer::set_ispb,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_code",
                &app::model::payouts::PixAccountBankTransfer::get_bank_code,
                &app::model::payouts::PixAccountBankTransfer::set_bank_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_account_type",
                &app::model::payouts::PixAccountBankTransfer::get_bank_account_type,
                &app::model::payouts::PixAccountBankTransfer::set_bank_account_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payouts::PixAccountBankTransfer::get_account_holder_name,
                &app::model::payouts::PixAccountBankTransfer::set_account_holder_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pix_account_bank_transfer";
    }
};

template<>
struct serde::Serializable<app::model::payouts::PixKeyBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "pix_key",
            &app::model::payouts::PixKeyBankTransfer::get_pix_key,
            &app::model::payouts::PixKeyBankTransfer::set_pix_key,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "pix_key_bank_transfer";
    }
};

template<>
struct serde::Serializable<app::model::payouts::PixEmvBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "emv",
            &app::model::payouts::PixEmvBankTransfer::get_emv,
            &app::model::payouts::PixEmvBankTransfer::set_emv,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "pix_emv_bank_transfer";
    }
};
