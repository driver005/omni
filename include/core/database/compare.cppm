export module core_database:compare;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief Comparison operators understood by the query builder. */
enum class Compare : std::uint8_t
{
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    LIKE,
    IN
};

/** @brief Instance functor mapping a Compare to its SQL operator. */
export class CompareSql
{
public:
    [[nodiscard]] constexpr std::string_view operator()(Compare comparison) const noexcept
    {
        switch (comparison) {
            case Compare::EQ:
                return "=";
            case Compare::NE:
                return "<>";
            case Compare::LT:
                return "<";
            case Compare::LE:
                return "<=";
            case Compare::GT:
                return ">";
            case Compare::GE:
                return ">=";
            case Compare::LIKE:
                return "LIKE";
            case Compare::IN:
                return "IN";
        }
        return "=";
    }
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_compare"> core_db_compare_suite = []
{
    "maps every comparison to SQL"_test = []
    {
        expect(CompareSql{}(Compare::EQ) == "=");
        expect(CompareSql{}(Compare::NE) == "<>");
        expect(CompareSql{}(Compare::GE) == ">=");
        expect(CompareSql{}(Compare::LIKE) == "LIKE");
        expect(CompareSql{}(Compare::IN) == "IN");
    };
};

} // namespace core::db::tests
#endif
