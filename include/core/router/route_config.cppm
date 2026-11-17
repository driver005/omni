export module core_router:route_config;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::router {

/** @brief Declarative route configuration entry loaded from app config. */
class RouteConfigEntry
{
public:
    RouteConfigEntry() = default;

    RouteConfigEntry(std::string path, std::string method, bool enabled) :
        m_path{std::move(path)},
        m_method{std::move(method)},
        m_enabled{enabled}
    {
    }

    [[nodiscard]] const std::string& path() const noexcept
    {
        return m_path;
    }

    [[nodiscard]] const std::string& method() const noexcept
    {
        return m_method;
    }

    [[nodiscard]] bool enabled() const noexcept
    {
        return m_enabled;
    }

    [[nodiscard]] std::size_t config_count() const noexcept
    {
        return m_config.size();
    }

    RouteConfigEntry& set(std::string key, std::string value)
    {
        m_config.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] std::optional<std::string> get(std::string_view key) const
    {
        const auto it = m_config.find(std::string{key});
        return it == m_config.end() ? std::nullopt : std::optional<std::string>{it->second};
    }

private:
    std::string m_path{};
    std::string m_method{"GET"};
    bool m_enabled{true};
    std::map<std::string, std::string> m_config{};
};

} // namespace core::router

#ifdef CONGELADO_TEST
namespace core::router::tests {
using namespace boost::ut;

suite<"core_router_route_config"> core_router_route_config_suite = []
{
    "carries declarative route config"_test = []
    {
        const RouteConfigEntry entry{"/posts", "GET", true};
        expect(entry.path() == "/posts");
        expect(entry.method() == "GET");
        expect(entry.enabled());
        RouteConfigEntry configured{"/posts", "GET", true};
        configured.set("auth", "true");
        expect(configured.get("auth") == std::string{"true"});
        expect(configured.config_count() == 1);
    };
};

} // namespace core::router::tests
#endif
