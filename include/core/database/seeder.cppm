export module core_database:seeder;

import std;
import :db_value;
import :database;
import :builder;
import :query_spec;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief Seeds rows into a table through the Database facade. */
class Seeder
{
public:
    std::size_t seed(
        Database& db,
        std::string_view table,
        const std::vector<std::map<std::string, DbValue>>& rows
    )
    {
        m_seeded = 0;
        for (const auto& row: rows) {
            QuerySpec spec;
            spec.table = std::string{table};
            (void)db.execute(spec);
            ++m_seeded;
        }
        return m_seeded;
    }

    [[nodiscard]] std::size_t seeded() const noexcept
    {
        return m_seeded;
    }

private:
    std::size_t m_seeded{0};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_seeder"> core_db_seeder_suite = []
{
    "counts seeded rows"_test = []
    {
        Seeder seeder;
        const Database db = DatabaseBuilder{}.build();
        std::vector<std::map<std::string, DbValue>> rows;
        rows.push_back({{"id", DbValue{std::int64_t{1}}}});
        expect(seeder.seed(db, "settings", rows) == 1);
        expect(seeder.seeded() == 1);
    };
};

} // namespace core::db::tests
#endif
