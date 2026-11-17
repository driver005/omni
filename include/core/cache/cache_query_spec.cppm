export module core_cache:cache_query_spec;

import std;
import :cache_entry;
import :cache_predicate;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Plain, resource-free description of a cache lookup. */
class CacheQuerySpec
{
public:
    std::string prefix{};
    std::vector<std::string> required_tags{};
    std::vector<CachePredicate> predicates{};
    std::optional<std::size_t> limit_count{};

    /** @brief True when an entry satisfies every constraint in this spec. */
    [[nodiscard]] bool matches(const CacheEntry& entry) const noexcept
    {
        if (entry.expired()) {
            return false;
        }
        if (!prefix.empty() && !entry.key.starts_with(prefix)) {
            return false;
        }
        for (const std::string& tag: required_tags) {
            if (!entry.has_tag(tag)) {
                return false;
            }
        }
        for (const CachePredicate& predicate: predicates) {
            if (!predicate.matches(entry)) {
                return false;
            }
        }
        return true;
    }
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_query_spec"> core_cache_query_spec_suite = []
{
    "filters by prefix, tags and predicates"_test = []
    {
        CacheEntry entry;
        entry.key = "users:v1:7";
        entry.attributes["age"] = "30";
        entry.tags.emplace_back("admin");

        CacheQuerySpec spec;
        spec.prefix = "users";
        spec.required_tags.emplace_back("admin");
        spec.predicates.emplace_back(CachePredicate{"age", CacheCompare::GT, "18"});
        expect(spec.matches(entry));

        spec.required_tags.emplace_back("guest");
        expect(!spec.matches(entry));
        spec.required_tags.pop_back();
        spec.prefix = "orders";
        expect(!spec.matches(entry));
    };
};

} // namespace core::cache::tests
#endif
