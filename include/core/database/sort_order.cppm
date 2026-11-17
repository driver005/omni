export module core_database:sort_order;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief Direction of an ORDER BY clause. */
enum class SortOrder : std::uint8_t
{
    ASC,
    DESC
};

/** @brief Instance functor mapping a SortOrder to its SQL clause. */
export class SortOrderSql
{
public:
    [[nodiscard]] constexpr std::string_view operator()(SortOrder order) const noexcept
    {
        return order == SortOrder::DESC ? " DESC" : " ASC";
    }
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_sort_order"> core_db_sort_order_suite = []
{
    "maps ordering to SQL"_test = []
    {
        expect(SortOrderSql{}(SortOrder::ASC) == " ASC");
        expect(SortOrderSql{}(SortOrder::DESC) == " DESC");
    };
};

} // namespace core::db::tests
#endif
