export module core_database:migration;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief One versioned schema migration with up/down SQL. */
class Migration
{
public:
    Migration() = default;

    Migration(std::uint64_t version, std::string name, std::string up_sql, std::string down_sql) :
        m_version{version},
        m_name{std::move(name)},
        m_up_sql{std::move(up_sql)},
        m_down_sql{std::move(down_sql)}
    {
    }

    [[nodiscard]] std::uint64_t version() const noexcept
    {
        return m_version;
    }

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& up_sql() const noexcept
    {
        return m_up_sql;
    }

    [[nodiscard]] const std::string& down_sql() const noexcept
    {
        return m_down_sql;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return m_version > 0 && !m_up_sql.empty();
    }

private:
    std::uint64_t m_version{0};
    std::string m_name{};
    std::string m_up_sql{};
    std::string m_down_sql{};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_migration"> core_db_migration_suite = []
{
    "carries version and SQL"_test = []
    {
        const Migration migration{1, "init", "CREATE TABLE t (id INT)", "DROP TABLE t"};
        expect(migration.valid());
        expect(migration.version() == 1);
        expect(migration.up_sql().contains("CREATE TABLE"));
        expect(migration.name() == "init");
    };
};

} // namespace core::db::tests
#endif
