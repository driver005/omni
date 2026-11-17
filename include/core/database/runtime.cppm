export module core_database:runtime;

import std;
import :storage_engine;
import :driver;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {
class DatabaseRuntime;
}

namespace core::db {

/**
 * @brief Internal state owned by the facade: the driver plus the set of tables
 * registered at build time. Kept in an implementation partition so consumers
 * can never reach the driver directly.
 */
class DatabaseRuntime
{
public:
    explicit DatabaseRuntime(std::unique_ptr<StorageBackend> backend) :
        m_driver{std::move(backend)}
    {
        m_driver.open();
    }

    DatabaseRuntime(const DatabaseRuntime&) = delete;
    DatabaseRuntime& operator=(const DatabaseRuntime&) = delete;
    DatabaseRuntime(DatabaseRuntime&&) noexcept = default;
    DatabaseRuntime& operator=(DatabaseRuntime&&) noexcept = default;

    void register_table(std::string_view table)
    {
        if (!m_tables.contains(std::string{table})) {
            m_tables.emplace(table, true);
            m_driver.create_table(table);
        }
    }

    [[nodiscard]] bool table_registered(std::string_view table) const noexcept
    {
        return m_tables.contains(std::string{table});
    }

    void require_table(std::string_view table) const
    {
        if (!table_registered(table)) {
            throw std::runtime_error(
                std::format("core_database: entity '{}' was not registered at build time", table)
            );
        }
    }

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_driver.is_open();
    }

    [[nodiscard]] std::string_view name() const noexcept
    {
        return m_driver.name();
    }

    [[nodiscard]] Driver& driver() noexcept
    {
        return m_driver;
    }

private:
    Driver m_driver;
    std::map<std::string, bool> m_tables{};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_database_runtime"> core_db_database_runtime_suite = []
{
    "registers tables and guards unregistered ones"_test = []
    {
        DatabaseRuntime runtime{StorageEngine::IN_MEMORY};
        expect(runtime.is_open());
        expect(!runtime.table_registered("users"));
        runtime.register_table("users");
        expect(runtime.table_registered("users"));
        expect(not throws<std::runtime_error>([&] { runtime.require_table("users"); }));
        expect(throws<std::runtime_error>([&] { runtime.require_table("ghosts"); }));
        expect(runtime.name() == "in_memory");
    };
};

} // namespace core::db::tests
#endif
