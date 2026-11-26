export module app_payment:telephone_order_result;

import std;
import core_serde;

export namespace app::handlers::telephone_order {

/**
 * @brief Twilio charge outcome wire document. All four fields are present on the
 * wire; a missing value is `null`. The Serializable field order below is the
 * upstream key order (charge_id, error_code, error_message, parameters).
 */
class TelephoneOrderChargeResult
{
public:
    void set_charge_id(std::optional<std::string> value)
    {
        charge_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_charge_id() const noexcept
    {
        return charge_id;
    }

    void set_error_code(std::optional<std::string> value)
    {
        error_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_code() const noexcept
    {
        return error_code;
    }

    void set_error_message(std::optional<std::string> value)
    {
        error_message = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_error_message() const noexcept
    {
        return error_message;
    }

    void set_parameters(std::unordered_map<std::string, std::string> value)
    {
        parameters = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::string>&
    get_parameters() const noexcept
    {
        return parameters;
    }

private:
    std::optional<std::string> charge_id{};
    std::optional<std::string> error_code{};
    std::optional<std::string> error_message{};
    std::unordered_map<std::string, std::string> parameters{};
};

} // namespace app::handlers::telephone_order

template<>
struct serde::Serializable<::app::handlers::telephone_order::TelephoneOrderChargeResult>
{
    static constexpr auto fields()
    {
        using Wire = ::app::handlers::telephone_order::TelephoneOrderChargeResult;
        return std::tuple{
            serde::FieldDesc<
                "charge_id",
                &Wire::get_charge_id,
                &Wire::set_charge_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_code",
                &Wire::get_error_code,
                &Wire::set_error_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "error_message",
                &Wire::get_error_message,
                &Wire::set_error_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "parameters",
                &Wire::get_parameters,
                &Wire::set_parameters,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "telephone_order_charge_result";
    }
};
