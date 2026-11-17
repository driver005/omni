export module blog_routesettings:routesettings;

import std;
import core_cache;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::routesettings {

/** @brief Route-settings service with cache-aside over the core cache facade. */
class RouteSettingsService
{
public:
    RouteSettingsService() :
        m_cache{core::cache::CacheBuilder{}.build()}
    {
    }

    void set(std::string key, std::string value)
    {
        m_cache.put(key, core::cache::to_cache_value(std::move(value)));
    }

    [[nodiscard]] std::optional<std::string> get(std::string_view key) const
    {
        const auto result = m_cache.get(key);
        if (!result.hit) return std::nullopt;
        return core::cache::value_as<std::string>(result.value);
    }

private:
    core::cache::Cache m_cache;
};

} // namespace blog::routesettings
#ifdef CONGELADO_TEST
namespace blog::routesettings::tests {
using namespace boost::ut;
suite<"blog_routesettings_routesettings"> blog_routesettings_routesettings_suite = []
{
    "route settings placeholder"_test = [] { expect(true); };
};
} // namespace blog::routesettings::tests
#endif
