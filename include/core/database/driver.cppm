export module core_database:driver;

import std;
import :storage_engine;
import :db_value;
import :query_result;
import :query_spec;
import :storage_backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::db {

/**
 * @brief Owns the active backend. This is the lowest layer the facade touches;
 * nothing here is re-exported by the module's primary interface.
 */
class Driver
{
public:
    explicit Driver(std::unique_ptr<StorageBackend> backend) :
        m_backend{std::move(backend)}
    {
    }

    Driver(const Driver&) = delete;
    Driver& operator=(const Driver&) = delete;
    Driver(Driver&&) noexcept = default;
    Driver& operator=(Driver&&) noexcept = default;

    void open()
    {
        if (m_backend) {
            m_backend->open();
        }
    }

    void close()
    {
        if (m_backend) {
            m_backend->close();
        }
    }

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_backend && m_backend->is_open();
    }

    [[nodiscard]] std::string_view name() const noexcept
    {
        return m_backend ? m_backend->name() : "none";
    }

    void create_table(std::string_view table)
    {
        m_backend->create_table(table);
    }

    [[nodiscard]] QueryResult run(const QuerySpec& spec)
    {
        return m_backend->run(spec);
    }

    std::uint64_t insert(std::string_view table, const std::map<std::string, DbValue>& values)
    {
        return m_backend->insert(table, values);
    }

    std::uint64_t update(
        std::string_view table,
        const std::map<std::string, DbValue>& values,
        std::string_view key,
        const DbValue& key_value
    )
    {
        return m_backend->update(table, values, key, key_value);
    }

    std::uint64_t remove(std::string_view table, std::string_view key, const DbValue& key_value)
    {
        return m_backend->remove(table, key, key_value);
    }

    /** @brief Monotonic id source used by the facade for auto-keyed entities. */
    [[nodiscard]] std::uint64_t next_id() noexcept
    {
        return m_next_id++;
    }

private:
    std::unique_ptr<StorageBackend> m_backend{};
    std::uint64_t m_next_id{1};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_driver"> core_db_driver_suite = []
{
    "selects and drives the in-memory backend"_test = []
    {
        Driver driver{StorageEngine::IN_MEMORY};
        driver.open();
        expect(driver.is_open());
        expect(driver.name() == "in_memory");
        driver.create_table("t");
        expect(driver.next_id() == 1);
        expect(driver.next_id() == 2);
        driver.close();
        expect(!driver.is_open());
    };

    "postgres engine is rejected until a plugin registers"_test = []
    {
        expect(
            throws<std::runtime_error>(
                []
                {
                    Driver driver{StorageEngine::POSTGRES};
                }
            )
        );
    };
};

} // namespace core::db::tests
#endif
