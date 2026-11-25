export module core_cache:compare;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Comparison operators for typed cache queries. */
enum class CacheCompare : std::uint8_t
{
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    HAS_TAG
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_compare"> core_cache_compare_suite = []
{
    "comparison enum preserves distinct values"_test = []
    {
        expect(
            static_cast<std::uint8_t>(CacheCompare::EQ) !=
            static_cast<std::uint8_t>(CacheCompare::NE)
        );
        expect(
            static_cast<std::uint8_t>(CacheCompare::HAS_TAG) ==
            static_cast<std::uint8_t>(CacheCompare::HAS_TAG)
        );
    };
};

} // namespace core::cache::tests
#endif
