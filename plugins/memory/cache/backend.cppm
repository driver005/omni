export module memory_backends:cache;

import std;
import core_cache;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cache {

/** @brief Reference backend: in-process store with optional LRU/LFU eviction. */
class LocalCacheBackend final : public CacheStorageBackend
{
public:
    explicit LocalCacheBackend(
        std::size_t capacity = 1024,
        EvictionPolicy policy = EvictionPolicy::LRU
    ) :
        m_capacity{capacity},
        m_policy{policy}
    {
    }

    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "local";
    }

    [[nodiscard]] bool is_open() const noexcept override
    {
        return m_open;
    }

    void open() override
    {
        m_open = true;
    }

    void close() override
    {
        m_open = false;
        m_store.clear();
    }

    [[nodiscard]] CacheEntry get(std::string_view key) const override
    {
        const auto it = m_store.find(std::string{key});
        if (it == m_store.end()) {
            return CacheEntry{.key = std::string{key}, .value = Miss{}};
        }
        if (it->second.expired()) {
            return CacheEntry{.key = std::string{key}, .value = Miss{}};
        }
        return it->second;
    }

    void put(const CacheEntry& entry) override
    {
        CacheEntry stored = entry;
        stored.stamp = ++m_clock;
        if (m_capacity != 0 && m_store.size() >= m_capacity &&
            !m_store.contains(stored.key)) {
            evict_one();
        }
        m_store.insert_or_assign(stored.key, std::move(stored));
    }

    bool erase(std::string_view key) override
    {
        return m_store.erase(std::string{key}) != 0;
    }

    void clear() override
    {
        m_store.clear();
    }

    [[nodiscard]] std::vector<CacheEntry> entries() const override
    {
        std::vector<CacheEntry> result;
        result.reserve(m_store.size());
        const auto now = CacheEntry::Clock::now();
        for (const auto& [key, entry]: m_store) {
            if (!entry.expired(now)) {
                result.push_back(entry);
            }
        }
        return result;
    }

    [[nodiscard]] std::size_t size() const noexcept override
    {
        return m_store.size();
    }

private:
    void evict_one()
    {
        auto victim = m_store.end();
        for (auto it = m_store.begin(); it != m_store.end(); ++it) {
            if (victim == m_store.end()) {
                victim = it;
                continue;
            }
            const bool worse =
                m_policy == EvictionPolicy::LFU ? it->second.hits < victim->second.hits
                                                : it->second.stamp < victim->second.stamp;
            if (worse) {
                victim = it;
            }
        }
        if (victim != m_store.end()) {
            m_store.erase(victim);
        }
    }

    std::map<std::string, CacheEntry> m_store{};
    std::size_t m_capacity{1024};
    EvictionPolicy m_policy{EvictionPolicy::LRU};
    std::uint64_t m_clock{0};
    bool m_open{false};
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_local_cache_backend"> core_cache_local_cache_backend_suite = []
{
    "stores, retrieves and evicts entries"_test = []
    {
        LocalCacheBackend backend{2, EvictionPolicy::LRU};
        backend.open();
        expect(backend.is_open());
        expect(backend.name() == "local");

        CacheEntry a;
        a.key = "a";
        a.value = to_cache_value(std::int64_t{1});
        backend.put(a);
        CacheEntry b;
        b.key = "b";
        b.value = to_cache_value(std::int64_t{2});
        backend.put(b);
        expect(backend.size() == 2);
        expect(std::get<std::int64_t>(backend.get("a").value) == 1);

        CacheEntry c;
        c.key = "c";
        c.value = to_cache_value(std::int64_t{3});
        backend.put(c); // evicts least-recently-stamped ("a")
        expect(backend.size() == 2);
        expect(IsMiss{}(backend.get("a").value));
        expect(backend.erase("b"));
        expect(!backend.erase("missing"));
    };

    "expired entries are treated as misses"_test = []
    {
        LocalCacheBackend backend;
        backend.open();
        CacheEntry entry;
        entry.key = "k";
        entry.value = to_cache_value(std::string{"v"});
        entry.expires_at = CacheEntry::Clock::now() - std::chrono::seconds{1};
        backend.put(entry);
        expect(IsMiss{}(backend.get("k").value));
        expect(backend.entries().empty());
    };
};

} // namespace core::cache::tests
#endif
