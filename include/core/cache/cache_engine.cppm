export module core_cache:cache_engine;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Selectable cache backends; concrete engine classes stay internal. */
enum class CacheEngine : std::uint8_t
{
    LOCAL,
    /** @brief Reserved for the redis plugin wired behind the facade. */
    REDIS
};

[[nodiscard]] constexpr std::string_view cache_engine_name(CacheEngine engine) noexcept
{
    return engine == CacheEngine::REDIS ? "redis" : "local";
}

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_engine"> core_cache_engine_suite = []
{
    "exposes engine names only"_test = []
    {
        expect(cache_engine_name(CacheEngine::LOCAL) == "local");
        expect(cache_engine_name(CacheEngine::REDIS) == "redis");
    };
};

} // namespace core::cache::tests
#endif
