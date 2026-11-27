module;
export module app_connector:model_payshap_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class PayshapBankTransfer : public serde::Serializer<PayshapBankTransfer>
{
public:
    void set_bank_account_number(SecretValue value)
    {
        bank_account_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_bank_account_number() const noexcept
    {
        return bank_account_number;
    }

    void set_account_holder_name(std::optional<SecretValue> value)
    {
        account_holder_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_account_holder_name() const noexcept
    {
        return account_holder_name;
    }

    void set_bank_name(std::optional<BankNamesValue> value)
    {
        bank_name = std::move(value);
    }

    [[nodiscard]] const std::optional<BankNamesValue>& get_bank_name() const noexcept
    {
        return bank_name;
    }

    SecretValue bank_account_number{};
    std::optional<SecretValue> account_holder_name{};
    std::optional<BankNamesValue> bank_name{};
};

class PayshapProxyBankTransfer : public serde::Serializer<PayshapProxyBankTransfer>
{
public:
    void set_cellphone(std::optional<SecretValue> value)
    {
        cellphone = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_cellphone() const noexcept
    {
        return cellphone;
    }

    void set_shap_id(std::optional<SecretValue> value)
    {
        shap_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_shap_id() const noexcept
    {
        return shap_id;
    }

    std::optional<SecretValue> cellphone{};
    std::optional<SecretValue> shap_id{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::PayshapBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "bank_account_number",
                &app::model::payouts::PayshapBankTransfer::get_bank_account_number,
                &app::model::payouts::PayshapBankTransfer::set_bank_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payouts::PayshapBankTransfer::get_account_holder_name,
                &app::model::payouts::PayshapBankTransfer::set_account_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_name",
                &app::model::payouts::PayshapBankTransfer::get_bank_name,
                &app::model::payouts::PayshapBankTransfer::set_bank_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payshap_bank_transfer";
    }
};

template<>
struct serde::Serializable<app::model::payouts::PayshapProxyBankTransfer>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "cellphone",
                &app::model::payouts::PayshapProxyBankTransfer::get_cellphone,
                &app::model::payouts::PayshapProxyBankTransfer::set_cellphone,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shap_id",
                &app::model::payouts::PayshapProxyBankTransfer::get_shap_id,
                &app::model::payouts::PayshapProxyBankTransfer::set_shap_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "payshap_proxy_bank_transfer";
    }
};
