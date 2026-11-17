export module core_database:string_literal;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db::reflect {

/** @brief Structural NTTP wrapper so a string literal can be a template argument. */
template<std::size_t N>
struct StringLiteral
{
    char value[N]{};

    consteval StringLiteral(const char (&source)[N]) noexcept
    {
        std::copy_n(source, N, value);
    }

    [[nodiscard]] constexpr std::string_view view() const noexcept
    {
        return {value, N - 1};
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept
    {
        return view();
    }

    template<std::size_t M>
    [[nodiscard]] constexpr bool operator==(const StringLiteral<M>& other) const noexcept
    {
        if constexpr (N != M) {
            return false;
        } else {
            for (std::size_t i = 0; i < N; ++i) {
                if (value[i] != other.value[i]) {
                    return false;
                }
            }
            return true;
        }
    }
};

} // namespace core::db::reflect

#ifdef CONGELADO_TEST
namespace core::db::reflect::tests {
using namespace boost::ut;

suite<"core_db_string_literal"> core_db_string_literal_suite = []
{
    "wraps a literal and compares structurally"_test = []
    {
        constexpr StringLiteral name{"users"};
        expect(name.view() == "users");
        expect(name == StringLiteral{"users"});
        expect(!(name == StringLiteral{"user"}));
    };
};

} // namespace core::db::reflect::tests
#endif
