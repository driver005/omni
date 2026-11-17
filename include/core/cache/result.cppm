export module core_cache:result;

import std;
import :value;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Outcome of a lookup: hit flag plus the value and its metadata. */
class CacheResult
{
public:
    bool hit{false};
    CacheValue value{Miss{}};
    std::optional<std::string> key{};
    std::vector<std::string> tags{};
    std::map<std::string, std::string> attributes{};

    [[nodiscard]] CacheResult miss(std::string_view requested_key)
    {
        CacheResult result;
        result.key = std::string{requested_key};
        return result;
    }

    [[nodiscard]] CacheResult found(
        std::string_view found_key,
        CacheValue found_value,
        std::vector<std::string> found_tags = {}
    )
    {
        CacheResult result;
        result.hit = true;
        result.key = std::string{found_key};
        result.value = std::move(found_value);
        result.tags = std::move(found_tags);
        return result;
    }

    template<typename T>
    [[nodiscard]] T value_or(T fallback) const
    {
        return hit ? value_as<T>(value, std::move(fallback)) : std::move(fallback);
    }
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_result"> core_cache_result_suite = []
{
    "miss and found carry the right state"_test = []
    {
        const CacheResult missed = CacheResult{}.miss("k");
        expect(!missed.hit);
        expect(missed.key == "k");
        expect(missed.value_or<std::int64_t>(9) == 9);

        const CacheResult found =
            CacheResult{}.found("k", to_cache_value(std::int64_t{5}), {"tag"});
        expect(found.hit);
        expect(found.value_or<std::int64_t>(0) == 5);
        expect(found.tags.size() == 1);
    };
};

} // namespace core::cache::tests
#endif
