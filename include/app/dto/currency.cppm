module;
export module app_dto:currency;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::currency {

class CurrencyConversionResponse
{
public:
    void set_converted_amount(std::string value) { converted_amount = std::move(value); }
    [[nodiscard]] const std::string& get_converted_amount() const noexcept { return converted_amount; }
    void set_currency(std::string value) { currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return currency; }
    std::string converted_amount{};
    std::string currency{};
};

class MinorUnitValue
{
public:
    void set_value(std::string v) { value = std::move(v); }
    [[nodiscard]] const std::string& get_value() const noexcept { return value; }
    std::string value{};
};

class CurrencyConversionParams
{
public:
    void set_amount(MinorUnitValue value) { amount = std::move(value); }
    [[nodiscard]] const MinorUnitValue& get_amount() const noexcept { return amount; }
    void set_to_currency(std::string value) { to_currency = std::move(value); }
    [[nodiscard]] const std::string& get_to_currency() const noexcept { return to_currency; }
    void set_from_currency(std::string value) { from_currency = std::move(value); }
    [[nodiscard]] const std::string& get_from_currency() const noexcept { return from_currency; }
    MinorUnitValue amount{};
    std::string to_currency{};
    std::string from_currency{};
};
}

template<>
struct serde::Serializable<app::dto::currency::CurrencyConversionParams>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"amount", &app::dto::currency::CurrencyConversionParams::get_amount, &app::dto::currency::CurrencyConversionParams::set_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"to_currency", &app::dto::currency::CurrencyConversionParams::get_to_currency, &app::dto::currency::CurrencyConversionParams::set_to_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"from_currency", &app::dto::currency::CurrencyConversionParams::get_from_currency, &app::dto::currency::CurrencyConversionParams::set_from_currency, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "currency_conversion_params"; }
};

template<>
struct serde::Serializable<app::dto::currency::CurrencyConversionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"converted_amount", &app::dto::currency::CurrencyConversionResponse::get_converted_amount, &app::dto::currency::CurrencyConversionResponse::set_converted_amount, serde::FieldOptions{}>{},
            serde::FieldDesc<"currency", &app::dto::currency::CurrencyConversionResponse::get_currency, &app::dto::currency::CurrencyConversionResponse::set_currency, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "currency_conversion_response"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::currency::tests {
using namespace boost::ut;
suite<"app_dto_currency"> app_dto_currency_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::currency::CurrencyConversionParams>::fields())> > 0);
    };
};
}
#endif
