export module core_cache:stats;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Running counters and current occupancy of a cache instance. */
class CacheStats
{
public:
    std::uint64_t hits{0};
    std::uint64_t misses{0};
    std::uint64_t writes{0};
    std::uint64_t removes{0};
    std::uint64_t evictions{0};
    std::size_t size{0};

    [[nodiscard]] std::uint64_t lookups() const noexcept
    {
        return hits + misses;
    }

    [[nodiscard]] double hit_rate() const noexcept
    {
        const std::uint64_t total = lookups();
        return total == 0 ? 0.0 : static_cast<double>(hits) / static_cast<double>(total);
    }
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_stats"> core_cache_stats_suite = []
{
    "hit rate is computed from lookups"_test = []
    {
        CacheStats stats;
        expect(stats.hit_rate() == 0.0);
        stats.hits = 3;
        stats.misses = 1;
        expect(stats.lookups() == 4);
        expect(stats.hit_rate() == 0.75);
    };
};

} // namespace core::cache::tests
#endif
