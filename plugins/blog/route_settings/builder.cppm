export module blog_routesettings:builder;
import std;
import :routesettings;
export namespace blog::routesettings {
class RouteSettingsServiceBuilder
{
public:
    [[nodiscard]] RouteSettingsService build() const { return RouteSettingsService{}; }
};
} // namespace blog::routesettings
