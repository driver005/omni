export module core_database:query_result;

import std;
import :row;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief Result of any statement that passed through the facade. */
class QueryResult
{
public:
    enum class Status : std::uint8_t
    {
        OK,
        NOOP
    };

    std::vector<Row> rows{};
    std::uint64_t rows_affected{0};
    Status status{Status::OK};

    [[nodiscard]] bool ok() const noexcept
    {
        return status == Status::OK;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return rows.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return rows.empty();
    }
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_query_result"> core_db_query_result_suite = []
{
    "reports status, rows and affected count"_test = []
    {
        QueryResult result;
        expect(result.ok());
        expect(result.empty());
        result.rows.emplace_back();
        result.rows_affected = 1;
        expect(result.size() == 1);
        expect(!result.empty());
        expect(result.rows_affected == 1);
    };
};

} // namespace core::db::tests
#endif
