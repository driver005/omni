export module core_database:query_spec;

import std;
import :predicate;
import :sort_order;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief Fully materialized, resource-free query description. */
class QuerySpec
{
public:
    std::string table{};
    std::vector<std::string> columns{};
    std::vector<Predicate> predicates{};
    std::optional<std::pair<std::string, SortOrder>> ordering{};
    std::optional<std::size_t> limit_count{};
    std::optional<std::size_t> offset_count{};

    /** @brief Renders the statement the backend will intercept and execute. */
    [[nodiscard]] std::string to_sql() const
    {
        std::string out = "SELECT ";
        out += columns.empty() ? "*" : join(columns, ",");
        out += " FROM " + table;

        if (!predicates.empty()) {
            out += " WHERE ";
            for (std::size_t i = 0; i < predicates.size(); ++i) {
                out += (i == 0 ? "" : " AND ") + predicates[i].to_sql();
            }
        }
        if (ordering) {
            out += " ORDER BY " + ordering->first +
                   std::string{SortOrderSql{}(ordering->second)};
        }
        if (limit_count) {
            out += " LIMIT " + std::to_string(*limit_count);
        }
        if (offset_count) {
            out += " OFFSET " + std::to_string(*offset_count);
        }
        return out;
    }

private:
    [[nodiscard]] std::string join(
        const std::vector<std::string>& parts,
        std::string_view separator
    ) const
    {
        std::string out;
        for (std::size_t i = 0; i < parts.size(); ++i) {
            out += (i == 0 ? "" : std::string{separator}) + parts[i];
        }
        return out;
    }
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_query_spec"> core_db_query_spec_suite = []
{
    "renders a full SELECT statement"_test = []
    {
        QuerySpec spec;
        spec.table = "users";
        spec.columns = {"id", "name"};
        spec.predicates.push_back(Predicate{"age", Compare::GT, to_db_value(21)});
        spec.ordering = std::make_pair(std::string{"name"}, SortOrder::DESC);
        spec.limit_count = 5;
        spec.offset_count = 10;
        expect(spec.to_sql() ==
               "SELECT id,name FROM users WHERE age > 21 ORDER BY name DESC LIMIT 5 OFFSET 10");
    };
};

} // namespace core::db::tests
#endif
