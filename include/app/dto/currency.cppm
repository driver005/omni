module;
export module app_dto:currency;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::currency {

/**
 * @brief Local mirror of common_utils::types::MinorUnit(i64)
 * (common_utils/src/types.rs:426-432). The upstream tuple struct deserializes
 * directly from the query string as an integer; `from_string` performs that
 * parse and `get_amount_as_i64` is the exact upstream accessor.
 */
class MinorUnit
{
public:
    MinorUnit() = default;

    explicit MinorUnit(std::int64_t value) noexcept :
        m_value{value}
    {
    }

    void set_value(std::int64_t value) noexcept
    {
        m_value = value;
    }

    [[nodiscard]] std::int64_t get_value() const noexcept
    {
        return m_value;
    }

    /** @brief common_utils/src/types.rs:430-432. */
    [[nodiscard]] std::int64_t get_amount_as_i64() const noexcept
    {
        return m_value;
    }

    /** @brief serde integer deserialization of `amount` from the query string. */
    [[nodiscard]] static std::optional<MinorUnit> from_string(std::string_view text) noexcept
    {
        std::int64_t value{};
        const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
        if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
            return std::nullopt;
        }
        return MinorUnit{value};
    }

    std::int64_t m_value{};
};

class CurrencyConversionResponse
{
public:
    void set_converted_amount(std::string value)
    {
        converted_amount = std::move(value);
    }

    [[nodiscard]] const std::string& get_converted_amount() const noexcept
    {
        return converted_amount;
    }

    void set_currency(std::string value)
    {
        currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return currency;
    }

    std::string converted_amount{};
    std::string currency{};
};

class CurrencyConversionParams
{
public:
    void set_amount(MinorUnit value)
    {
        amount = value;
    }

    [[nodiscard]] const MinorUnit& get_amount() const noexcept
    {
        return amount;
    }

    void set_to_currency(std::string value)
    {
        to_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_to_currency() const noexcept
    {
        return to_currency;
    }

    void set_from_currency(std::string value)
    {
        from_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_from_currency() const noexcept
    {
        return from_currency;
    }

    MinorUnit amount{};
    std::string to_currency{};
    std::string from_currency{};
};
} // namespace app::dto::currency

template<>
struct serde::Serializable<app::dto::currency::MinorUnit>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "value",
                &app::dto::currency::MinorUnit::get_value,
                &app::dto::currency::MinorUnit::set_value,
                serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "minor_unit";
    }
};

template<>
struct serde::Serializable<app::dto::currency::CurrencyConversionParams>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "amount",
                &app::dto::currency::CurrencyConversionParams::get_amount,
                &app::dto::currency::CurrencyConversionParams::set_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "to_currency",
                &app::dto::currency::CurrencyConversionParams::get_to_currency,
                &app::dto::currency::CurrencyConversionParams::set_to_currency,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "from_currency",
                &app::dto::currency::CurrencyConversionParams::get_from_currency,
                &app::dto::currency::CurrencyConversionParams::set_from_currency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "currency_conversion_params";
    }
};

template<>
struct serde::Serializable<app::dto::currency::CurrencyConversionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "converted_amount",
                &app::dto::currency::CurrencyConversionResponse::get_converted_amount,
                &app::dto::currency::CurrencyConversionResponse::set_converted_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "currency",
                &app::dto::currency::CurrencyConversionResponse::get_currency,
                &app::dto::currency::CurrencyConversionResponse::set_currency,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "currency_conversion_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::currency::tests {
using namespace boost::ut;
suite<"app_dto_currency"> app_dto_currency_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::currency::CurrencyConversionParams>::fields())> > 0
        );
    };
    "minor unit parses i64 and exposes the upstream accessor"_test = []
    {
        const auto amount = MinorUnit::from_string("2000");
        expect(amount.has_value());
        expect(amount->get_amount_as_i64() == 2'000);
        expect(!MinorUnit::from_string("12.5").has_value());
    };
};
} // namespace app::dto::currency::tests
#endif
