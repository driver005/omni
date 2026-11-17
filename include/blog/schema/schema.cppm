export module blog_schema:schema;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::schema {

/** @brief Registry of Ghost schema migrations applied through core_database. */
class SchemaRegistry
{
public:
    void add(core::db::Migration migration)
    {
        m_runner.add(std::move(migration));
    }

    [[nodiscard]] bool apply_next(core::db::Database& db)
    {
        return m_runner.apply_next(db);
    }

    [[nodiscard]] std::uint64_t version() const noexcept
    {
        return m_runner.current_version();
    }

private:
    core::db::MigrationRunner m_runner{};
};

} // namespace blog::schema

#ifdef CONGELADO_TEST
namespace blog::schema::tests {
using namespace boost::ut;
suite<"blog_schema_schema"> blog_schema_schema_suite = []
{
    "registers migrations"_test = []
    {
        SchemaRegistry registry;
        registry.add(core::db::Migration{1, "init", "CREATE TABLE t (id INT)", "DROP TABLE t"});
        expect(registry.version() == 0);
    };
};
} // namespace blog::schema::tests
#endif
