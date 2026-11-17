export module core_cache:eviction_policy;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Policy applied when a bounded cache reaches capacity. */
enum class EvictionPolicy : std::uint8_t
{
    LRU,
    LFU,
    TTL_ONLY
};

[[nodiscard]] constexpr std::string_view eviction_policy_name(EvictionPolicy policy) noexcept
{
    switch (policy) {
        case EvictionPolicy::LRU:
            return "lru";
        case EvictionPolicy::LFU:
            return "lfu";
        case EvictionPolicy::TTL_ONLY:
            return "ttl_only";
    }
    return "lru";
}

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_eviction_policy"> core_cache_eviction_policy_suite = []
{
    "names every policy"_test = []
    {
        expect(eviction_policy_name(EvictionPolicy::LRU) == "lru");
        expect(eviction_policy_name(EvictionPolicy::LFU) == "lfu");
        expect(eviction_policy_name(EvictionPolicy::TTL_ONLY) == "ttl_only");
    };
};

} // namespace core::cache::tests
#endif
