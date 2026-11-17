export module core_cache:cache_predicate;

import std;
import :cache_compare;
import :cache_entry;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief One predicate evaluated against a cache entry's attributes or tags. */
class CachePredicate
{
public:
    std::string field{};
    CacheCompare comparison{CacheCompare::EQ};
    std::string value{};

    [[nodiscard]] bool matches(const CacheEntry& entry) const noexcept
    {
        if (comparison == CacheCompare::HAS_TAG) {
            return entry.has_tag(value);
        }
        const auto it = entry.attributes.find(field);
        if (it == entry.attributes.end()) {
            return false;
        }
        return compare_strings(it->second, value, comparison);
    }

    [[nodiscard]] std::string to_sql_like() const
    {
        static constexpr std::string_view OPS[] = {"=", "<>", "<", "<=", ">", ">=", "HAS_TAG"};
        return field + " " + std::string{OPS[static_cast<std::size_t>(comparison)]} + " " + value;
    }

private:
    [[nodiscard]] static bool compare_strings(
        std::string_view lhs,
        std::string_view rhs,
        CacheCompare comparison
    ) noexcept
    {
        const int cmp = lhs.compare(rhs);
        switch (comparison) {
            case CacheCompare::EQ:
                return cmp == 0;
            case CacheCompare::NE:
                return cmp != 0;
            case CacheCompare::LT:
                return cmp < 0;
            case CacheCompare::LE:
                return cmp <= 0;
            case CacheCompare::GT:
                return cmp > 0;
            case CacheCompare::GE:
                return cmp >= 0;
            case CacheCompare::HAS_TAG:
                return false;
        }
        return false;
    }
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_predicate"> core_cache_predicate_suite = []
{
    "matches attributes and tags"_test = []
    {
        CacheEntry entry;
        entry.attributes["age"] = "30";
        entry.tags.emplace_back("admin");

        const CachePredicate eq{"age", CacheCompare::GE, "18"};
        expect(eq.matches(entry));
        const CachePredicate ne{"age", CacheCompare::EQ, "18"};
        expect(!ne.matches(entry));
        const CachePredicate tag{"", CacheCompare::HAS_TAG, "admin"};
        expect(tag.matches(entry));
        expect(eq.to_sql_like() == "age >= 18");
    };
};

} // namespace core::cache::tests
#endif
