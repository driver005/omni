export module core_cache:aside;

import std;
import :value;
import :entry;
import :cache;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/**
 * @brief Read-through/write-aside helper: returns a cached value or loads and
 * stores it through the opaque Cache facade. Holds a Cache reference only.
 */
template<typename T>
class CacheAside
{
public:
    explicit CacheAside(Cache& cache) :
        m_cache{cache}
    {
    }

    template<typename Loader>
    [[nodiscard]] T get_or_load(std::string_view key, Loader&& loader, Ttl ttl = Ttl{}.never())
    {
        const CacheResult result = m_cache.get(key);
        if (result.hit) {
            return value_as<T>(result.value);
        }
        const T value = loader();
        m_cache.put(key, to_cache_value(value), std::move(ttl));
        return value;
    }

    [[nodiscard]] std::optional<T> load(std::string_view key) const
    {
        const CacheResult result = m_cache.get(key);
        if (!result.hit) {
            return std::nullopt;
        }
        return value_as<T>(result.value);
    }

    void store(std::string_view key, const T& value, Ttl ttl = Ttl{}.never())
    {
        m_cache.put(key, to_cache_value(value), std::move(ttl));
    }

private:
    Cache& m_cache;
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_cache_aside"> core_cache_cache_aside_suite = []
{
    "loads once then serves from cache"_test = []
    {
        Cache cache;
        CacheAside<std::string> aside{cache};
        int calls{0};
        const auto first = aside.get_or_load("key", [&] {
            ++calls;
            return std::string{"value"};
        });
        expect(first == "value");
        expect(calls == 1);
        const auto second = aside.get_or_load("key", [&] {
            ++calls;
            return std::string{"other"};
        });
        expect(second == "value");
        expect(calls == 1);
        expect(aside.load("key") == std::string{"value"});
    };
};

} // namespace core::cache::tests
#endif
