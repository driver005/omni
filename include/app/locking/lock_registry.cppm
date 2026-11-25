module;
export module app_locking:lock_registry;

import std;
import core_cache;
import :api_lock;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::locking {

/** @brief Redis-style mutual-exclusion locks backed by the injected cache with TTL. */
class LockRegistry
{
public:
    explicit LockRegistry(core::cache::Cache& cache) :
        m_cache{cache}
    {
    }

    /** @brief Acquires `key` when free, returning the lock snapshot; nullopt when already held. */
    std::optional<ApiLock>
    acquire(std::string_view key, core::cache::Ttl ttl, std::string owner = {})
    {
        const std::string cache_key = scoped(key);
        if (m_cache.get(cache_key).hit) {
            return std::nullopt;
        }

        ApiLock lock{};
        lock.set_key(std::string{key});
        lock.set_owner(std::move(owner));
        const std::int64_t now = now_seconds();
        lock.set_acquired_at(now);
        lock.set_expires_at(expiry_seconds(now, ttl));
        m_cache.put(cache_key, core::cache::to_cache_value(lock.get_owner()), ttl);
        return lock;
    }

    /** @brief Convenience overload taking a TTL in seconds. */
    std::optional<ApiLock>
    acquire(std::string_view key, std::int64_t ttl_seconds, std::string owner = {})
    {
        return acquire(key, core::cache::Ttl{}.seconds(ttl_seconds), std::move(owner));
    }

    bool release(std::string_view key)
    {
        return m_cache.erase(scoped(key));
    }

    [[nodiscard]] bool is_held(std::string_view key) const
    {
        return m_cache.get(scoped(key)).hit;
    }

private:
    [[nodiscard]] std::string scoped(std::string_view key) const
    {
        return std::format("lock:{}", key);
    }

    [[nodiscard]] std::int64_t now_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    }

    [[nodiscard]] std::int64_t expiry_seconds(std::int64_t now, const core::cache::Ttl& ttl) const
    {
        if (ttl.is_never()) {
            return std::numeric_limits<std::int64_t>::max();
        }
        return now + std::chrono::duration_cast<std::chrono::seconds>(ttl.duration()).count();
    }

    core::cache::Cache& m_cache;
};

} // namespace app::locking

#ifdef CONGELADO_TEST
namespace app::locking::tests {

using namespace boost::ut;

suite<"app_locking_lock_registry"> lock_registry_suite = []
{
    "acquires, guards and releases a keyed lock"_test = []
    {
        core::cache::Cache cache = core::cache::CacheBuilder{}
                                       .engine(core::cache::CacheEngine::LOCAL)
                                       .capacity(64)
                                       .policy(core::cache::EvictionPolicy::LRU)
                                       .build();
        LockRegistry registry{cache};

        const auto lock = registry.acquire("payment:1", core::cache::Ttl{}.seconds(30), "handler");
        expect(lock.has_value());
        expect(lock->get_key() == "payment:1");
        expect(lock->get_owner() == "handler");
        expect(lock->get_expires_at() > lock->get_acquired_at());
        expect(registry.is_held("payment:1"));

        expect(!registry.acquire("payment:1", std::int64_t{30}).has_value());
        expect(registry.release("payment:1"));
        expect(!registry.is_held("payment:1"));
        expect(registry.acquire("payment:1", std::int64_t{30}).has_value());
    };
};

} // namespace app::locking::tests
#endif
