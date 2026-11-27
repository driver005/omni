module;
export module app_connector:model_paypal_payouts;

import std;
import core_serde;
import :model_shared_payouts;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payouts {

class Paypal : public serde::Serializer<Paypal>
{
public:
    void set_email(std::optional<EmailValue> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<EmailValue>& get_email() const noexcept
    {
        return email;
    }

    void set_telephone_number(std::optional<SecretValue> value)
    {
        telephone_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_telephone_number() const noexcept
    {
        return telephone_number;
    }

    void set_paypal_id(std::optional<SecretValue> value)
    {
        paypal_id = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_paypal_id() const noexcept
    {
        return paypal_id;
    }

    std::optional<EmailValue> email{};
    std::optional<SecretValue> telephone_number{};
    std::optional<SecretValue> paypal_id{};
};

} // namespace app::model::payouts

template<>
struct serde::Serializable<app::model::payouts::Paypal>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "email",
                &app::model::payouts::Paypal::get_email,
                &app::model::payouts::Paypal::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "telephone_number",
                &app::model::payouts::Paypal::get_telephone_number,
                &app::model::payouts::Paypal::set_telephone_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paypal_id",
                &app::model::payouts::Paypal::get_paypal_id,
                &app::model::payouts::Paypal::set_paypal_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "paypal";
    }
};
