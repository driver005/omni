export module core_cache:entry;

import std;
import :value;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Everything stored for one key: raw value, reflected attributes, TTL and tags. */
class CacheEntry
{
public:
    using Clock = std::chrono::system_clock;

    std::string key{};
    CacheValue value{Miss{}};
    std::map<std::string, std::string> attributes{};
    std::vector<std::string> tags{};
    Clock::time_point expires_at{Clock::time_point::max()};
    std::uint64_t version{0};
    std::uint64_t hits{0};
    std::uint64_t stamp{0};

    [[nodiscard]] bool expired(Clock::time_point now = Clock::now()) const noexcept
    {
        return expires_at != Clock::time_point::max() && now >= expires_at;
    }

    [[nodiscard]] bool has_tag(std::string_view tag) const noexcept
    {
        return std::ranges::find(tags, tag) != tags.end();
    }
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_entry"> core_cache_entry_suite = []
{
    "stores attributes and tags, tracks expiry"_test = []
    {
        CacheEntry entry;
        entry.key = "users:1";
        entry.value = to_cache_value(std::string{"Ada"});
        entry.attributes["name"] = "Ada";
        entry.tags.emplace_back("admin");
        expect(entry.key == "users:1");
        expect(entry.has_tag("admin"));
        expect(!entry.has_tag("guest"));
        expect(!entry.expired());
        entry.expires_at = CacheEntry::Clock::now() - std::chrono::seconds{1};
        expect(entry.expired());
    };
};

} // namespace core::cache::tests
#endif
