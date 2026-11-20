module;
export module app_db:migration;

import std;
import core_database;
import :schema;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::db {

/**
 * @brief Applies the schema registry through an injected core::db::Database.
 * Each table is registered once per runner; the underlying runtime's
 * register_table is itself idempotent. Running migrations twice therefore
 * neither fails nor duplicates rows.
 */
class MigrationRunner
{
public:
    MigrationRunner() = default;

    explicit MigrationRunner(SchemaRegistry registry) :
        m_registry{std::move(registry)}
    {
    }

    [[nodiscard]] const SchemaRegistry& registry() const noexcept
    {
        return m_registry;
    }

    /** @brief Registers every table not yet applied by this runner. */
    std::size_t migrate(core::db::Database& db)
    {
        for (const TableDescriptor& table : m_registry.tables()) {
            apply_table(db, table);
        }
        return m_applied.size();
    }

    /** @brief Registers one named table, returning false for an unknown name. */
    bool migrate_table(core::db::Database& db, std::string_view name)
    {
        const std::optional<TableDescriptor> table = m_registry.find(name);
        if (!table) {
            return false;
        }
        apply_table(db, *table);
        return true;
    }

    /** @brief Full DDL for a table, exposed for diagnostics and evidence. */
    [[nodiscard]] std::optional<std::string> ddl_for(std::string_view name) const
    {
        const std::optional<TableDescriptor> table = m_registry.find(name);
        if (!table) {
            return std::nullopt;
        }
        return DdlRenderer{}(*table);
    }

    [[nodiscard]] bool applied(std::string_view name) const noexcept
    {
        return m_applied.contains(std::string{name});
    }

    [[nodiscard]] std::size_t applied_count() const noexcept
    {
        return m_applied.size();
    }

    [[nodiscard]] std::vector<std::string> applied_tables() const
    {
        return {m_applied.begin(), m_applied.end()};
    }

private:
    void apply_table(core::db::Database& db, const TableDescriptor& table)
    {
        if (m_applied.contains(table.get_name())) {
            return;
        }
        // Rendered DDL is deterministic schema data; the backend keys tables by
        // name, so registration is the operation that makes the table usable.
        (void)DdlRenderer{}(table);
        db.register_table(table.get_name());
        m_applied.insert(table.get_name());
    }

    SchemaRegistry m_registry{};
    std::set<std::string> m_applied{};
};

} // namespace app::db

#ifdef CONGELADO_TEST
namespace app::db::tests {

using namespace boost::ut;

suite<"app_db_migration_runner"> migration_runner_suite = []
{
    "applies every registered table once"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}.build();
        MigrationRunner runner{};
        expect(runner.applied_count() == 0);
        const std::size_t count = runner.migrate(db);
        expect(count == runner.registry().size());
        expect(runner.applied("payment_intents"));
        expect(runner.applied("process_trackers"));
        expect(runner.applied("configs"));
    };

    "second migration is a no-op"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}.build();
        MigrationRunner runner{};
        const std::size_t first = runner.migrate(db);
        const std::size_t second = runner.migrate(db);
        expect(first == second);
        expect(runner.applied_count() == first);
    };

    "migrating the same table through a fresh runner is safe"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}.build();
        MigrationRunner first{};
        expect(first.migrate_table(db, "api_keys"));
        MigrationRunner second{};
        expect(second.migrate_table(db, "api_keys"));
        expect(!second.migrate_table(db, "unknown_table"));
        expect(second.ddl_for("api_keys")->contains("CREATE TABLE IF NOT EXISTS api_keys"));
    };
};

} // namespace app::db::tests
#endif
