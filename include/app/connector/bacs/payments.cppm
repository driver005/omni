module;
export module app_connector:model_bacs_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class BacsBankTransferInstructions : public serde::Serializer<BacsBankTransferInstructions>
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

    void set_account_number(SecretValue value)
    {
        account_number = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_account_number() const noexcept
    {
        return account_number;
    }

    void set_sort_code(SecretValue value)
    {
        sort_code = std::move(value);
    }

    [[nodiscard]] const SecretValue& get_sort_code() const noexcept
    {
        return sort_code;
    }

    SecretValue account_holder_name{};
    SecretValue account_number{};
    SecretValue sort_code{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::BacsBankTransferInstructions>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "account_holder_name",
                &app::model::payments::BacsBankTransferInstructions::get_account_holder_name,
                &app::model::payments::BacsBankTransferInstructions::set_account_holder_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_number",
                &app::model::payments::BacsBankTransferInstructions::get_account_number,
                &app::model::payments::BacsBankTransferInstructions::set_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "sort_code",
                &app::model::payments::BacsBankTransferInstructions::get_sort_code,
                &app::model::payments::BacsBankTransferInstructions::set_sort_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "bacs_bank_transfer_instructions";
    }
};
