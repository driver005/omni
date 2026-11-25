export module core_cache:driver;

import std;
import :engine;
import :value;
import :eviction_policy;
import :entry;
import :query_spec;
import :stats;
import :storage_backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cache {

/** @brief Owns the active backend and running stats. Internal to the module. */
class CacheDriver
{
public:
    explicit CacheDriver(std::unique_ptr<CacheStorageBackend> backend) :
        m_backend{std::move(backend)}
    {
    }

    CacheDriver(const CacheDriver&) = delete;
    CacheDriver& operator=(const CacheDriver&) = delete;
    CacheDriver(CacheDriver&&) noexcept = default;
    CacheDriver& operator=(CacheDriver&&) noexcept = default;

    void open()
    {
        m_backend->open();
    }

    void close()
    {
        m_backend->close();
    }

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_backend && m_backend->is_open();
    }

    [[nodiscard]] std::string_view name() const noexcept
    {
        return m_backend ? m_backend->name() : "none";
    }

    [[nodiscard]] CacheEntry get(std::string_view key)
    {
        CacheEntry entry = m_backend->get(key);
        if (!IsMiss{}(entry.value)) {
            ++m_stats.hits;
            entry.hits++;
            m_backend->put(entry);
        } else {
            ++m_stats.misses;
        }
        return entry;
    }

    void put(const CacheEntry& entry)
    {
        m_backend->put(entry);
        ++m_stats.writes;
        m_stats.size = m_backend->size();
    }

    bool erase(std::string_view key)
    {
        const bool removed = m_backend->erase(key);
        if (removed) {
            ++m_stats.removes;
            m_stats.size = m_backend->size();
        }
        return removed;
    }

    void clear()
    {
        m_backend->clear();
        m_stats.size = 0;
    }

    [[nodiscard]] std::vector<CacheEntry> query(const CacheQuerySpec& spec)
    {
        std::vector<CacheEntry> result;
        for (const CacheEntry& entry: m_backend->entries()) {
            if (spec.matches(entry)) {
                result.push_back(entry);
                if (spec.limit_count && result.size() >= *spec.limit_count) {
                    break;
                }
            }
        }
        return result;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_backend->size();
    }

    [[nodiscard]] const CacheStats& stats() const noexcept
    {
        return m_stats;
    }

private:
    std::unique_ptr<CacheStorageBackend> m_backend{};
    CacheStats m_stats{};
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_driver"> core_cache_driver_suite = []
{
    "drives backend and records stats"_test = []
    {
        CacheDriver driver{CacheEngine::LOCAL, 16, EvictionPolicy::LRU};
        driver.open();
        expect(driver.is_open());
        expect(driver.name() == "local");

        CacheEntry entry;
        entry.key = "k";
        entry.value = to_cache_value(std::string{"v"});
        driver.put(entry);
        expect(std::get<std::string>(driver.get("k").value) == "v");
        expect(IsMiss{}(driver.get("missing").value));
        expect(driver.stats().hits == 1);
        expect(driver.stats().misses == 1);
        expect(driver.size() == 1);
        expect(driver.erase("k"));
        driver.close();
        expect(!driver.is_open());
    };

    "redis engine is rejected until a plugin registers"_test = []
    {
        expect(
            throws<std::runtime_error>(
                []
                {
                    CacheDriver driver{CacheEngine::REDIS, 0, EvictionPolicy::LRU};
                }
            )
        );
    };
};

} // namespace core::cache::tests
#endif
