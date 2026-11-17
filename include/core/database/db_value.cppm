export module core_database:db_value;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief Explicit tag for a SQL NULL cell. */
struct Null
{
    constexpr bool operator==(const Null&) const noexcept = default;
};

/** @brief The single value type that may cross the abstraction boundary. */
using DbValue = std::variant<
    Null,
    std::int64_t,
    std::uint64_t,
    double,
    bool,
    std::string,
    std::vector<std::uint8_t>,
    std::vector<std::string>,
    std::chrono::system_clock::time_point>;

/**
 * @brief Normalizes a C++ value into a DbValue. Integral widths collapse to
 * std::int64_t so a reflected `int` member stores cleanly.
 */
template<typename T>
[[nodiscard]] DbValue to_db_value(T&& value)
{
    using U = std::remove_cvref_t<T>;
    if constexpr (std::same_as<U, bool>) {
        return DbValue{value};
    } else if constexpr (std::integral<U> && std::is_signed_v<U>) {
        return DbValue{static_cast<std::int64_t>(value)};
    } else if constexpr (std::integral<U>) {
        return DbValue{static_cast<std::uint64_t>(value)};
    } else if constexpr (std::floating_point<U>) {
        return DbValue{static_cast<double>(value)};
    } else if constexpr (std::same_as<U, std::string> || std::convertible_to<U, std::string_view>) {
        return DbValue{std::string{std::string_view{value}}};
    } else if constexpr (std::same_as<U, Null>) {
        return DbValue{Null{}};
    } else {
        return DbValue{std::forward<T>(value)};
    }
}

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_value"> core_db_value_suite = []
{
    "null and arithmetic normalization"_test = []
    {
        expect(std::holds_alternative<Null>(to_db_value(Null{})));
        expect(std::get<std::int64_t>(to_db_value(7)) == 7);
        expect(std::get<double>(to_db_value(2.5f)) == 2.5);
        expect(std::get<bool>(to_db_value(true)));
    };

    "string and blob normalization"_test = []
    {
        expect(std::get<std::string>(to_db_value(std::string{"ada"})) == "ada");
        const std::vector<std::uint8_t> blob{0x01, 0x02};
        expect(std::get<std::vector<std::uint8_t>>(to_db_value(blob)).size() == 2);
    };
};

} // namespace core::db::tests
#endif
