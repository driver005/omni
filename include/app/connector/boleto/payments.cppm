module;
export module app_connector:model_boleto_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

class BoletoVoucherData : public serde::Serializer<BoletoVoucherData>
{
public:
    void set_social_security_number(std::optional<SecretValue> value)
    {
        social_security_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_social_security_number() const noexcept
    {
        return social_security_number;
    }

    void set_bank_number(std::optional<SecretValue> value)
    {
        bank_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_bank_number() const noexcept
    {
        return bank_number;
    }

    void set_document_type(std::optional<DocumentKindValue> value)
    {
        document_type = std::move(value);
    }

    [[nodiscard]] const std::optional<DocumentKindValue>& get_document_type() const noexcept
    {
        return document_type;
    }

    void set_fine_percentage(std::optional<std::string> value)
    {
        fine_percentage = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_fine_percentage() const noexcept
    {
        return fine_percentage;
    }

    void set_fine_quantity_days(std::optional<std::string> value)
    {
        fine_quantity_days = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_fine_quantity_days() const noexcept
    {
        return fine_quantity_days;
    }

    void set_interest_percentage(std::optional<std::string> value)
    {
        interest_percentage = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_interest_percentage() const noexcept
    {
        return interest_percentage;
    }

    void set_write_off_quantity_days(std::optional<std::string> value)
    {
        write_off_quantity_days = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_write_off_quantity_days() const noexcept
    {
        return write_off_quantity_days;
    }

    void set_messages(std::optional<std::vector<std::string>> value)
    {
        messages = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_messages() const noexcept
    {
        return messages;
    }

    void set_due_date(std::optional<std::string> value)
    {
        due_date = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_due_date() const noexcept
    {
        return due_date;
    }

    std::optional<SecretValue> social_security_number{};
    std::optional<SecretValue> bank_number{};
    std::optional<DocumentKindValue> document_type{};
    std::optional<std::string> fine_percentage{};
    std::optional<std::string> fine_quantity_days{};
    std::optional<std::string> interest_percentage{};
    std::optional<std::string> write_off_quantity_days{};
    std::optional<std::vector<std::string>> messages{};
    std::optional<std::string> due_date{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::BoletoVoucherData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "social_security_number",
                &app::model::payments::BoletoVoucherData::get_social_security_number,
                &app::model::payments::BoletoVoucherData::set_social_security_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "bank_number",
                &app::model::payments::BoletoVoucherData::get_bank_number,
                &app::model::payments::BoletoVoucherData::set_bank_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "document_type",
                &app::model::payments::BoletoVoucherData::get_document_type,
                &app::model::payments::BoletoVoucherData::set_document_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fine_percentage",
                &app::model::payments::BoletoVoucherData::get_fine_percentage,
                &app::model::payments::BoletoVoucherData::set_fine_percentage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fine_quantity_days",
                &app::model::payments::BoletoVoucherData::get_fine_quantity_days,
                &app::model::payments::BoletoVoucherData::set_fine_quantity_days,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "interest_percentage",
                &app::model::payments::BoletoVoucherData::get_interest_percentage,
                &app::model::payments::BoletoVoucherData::set_interest_percentage,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "write_off_quantity_days",
                &app::model::payments::BoletoVoucherData::get_write_off_quantity_days,
                &app::model::payments::BoletoVoucherData::set_write_off_quantity_days,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "messages",
                &app::model::payments::BoletoVoucherData::get_messages,
                &app::model::payments::BoletoVoucherData::set_messages,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "due_date",
                &app::model::payments::BoletoVoucherData::get_due_date,
                &app::model::payments::BoletoVoucherData::set_due_date,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "boleto_voucher_data";
    }
};
