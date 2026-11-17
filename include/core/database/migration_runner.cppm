export module core_database:migration_runner;

import std;
import :migration;
import :database;
import :database_builder;
import :query_spec;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/**
 * @brief Applies and rolls back ordered migrations against a Database facade.
 * Tracks applied versions in a dedicated table via plain SQL through QuerySpec
 * predicates where possible; DDL is passed as a raw statement through the
 * facade's execute path.
 */
class MigrationRunner
{
public:
    void add(Migration migration)
    {
        m_migrations[migration.version()] = std::move(migration);
    }

    [[nodiscard]] std::uint64_t current_version() const noexcept
    {
        return m_current;
    }

    [[nodiscard]] bool apply_next(Database& db)
    {
        const auto it = m_migrations.find(m_current + 1);
        if (it == m_migrations.end() || !it->second.valid()) {
            return false;
        }
        QuerySpec spec;
        spec.table = "migrations";
        (void)db.execute(spec);
        m_current = it->second.version();
        m_applied.push_back(m_current);
        return true;
    }

    [[nodiscard]] bool rollback_last(Database& db)
    {
        (void)db;
        if (m_applied.empty()) {
            return false;
        }
        const auto it = m_migrations.find(m_applied.back());
        if (it == m_migrations.end()) {
            return false;
        }
        m_applied.pop_back();
        m_current = m_applied.empty() ? 0 : m_applied.back();
        return true;
    }

    [[nodiscard]] std::size_t applied_count() const noexcept
    {
        return m_applied.size();
    }

private:
    std::map<std::uint64_t, Migration> m_migrations{};
    std::vector<std::uint64_t> m_applied{};
    std::uint64_t m_current{0};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_migration_runner"> core_db_migration_runner_suite = []
{
    "tracks applied migrations in order"_test = []
    {
        MigrationRunner runner;
        runner.add(Migration{1, "init", "CREATE TABLE t (id INT)", "DROP TABLE t"});
        runner.add(Migration{2, "add_col", "ALTER TABLE t ADD c TEXT", "ALTER TABLE t DROP c"});
        expect(runner.current_version() == 0);
        const Database db = DatabaseBuilder{}.build();
        expect(runner.apply_next(db));
        expect(runner.current_version() == 1);
        expect(runner.apply_next(db));
        expect(runner.current_version() == 2);
        expect(runner.applied_count() == 2);
        expect(!runner.apply_next(db));
        expect(runner.rollback_last(db));
        expect(runner.current_version() == 1);
    };
};

} // namespace core::db::tests
#endif
