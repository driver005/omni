export module blog_settings:builder;

import std;
import :settings;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::settings {

/** @brief Fluent entry point for the Ghost settings service. */
class SettingsServiceBuilder
{
public:
    [[nodiscard]] SettingsService build() const
    {
        return SettingsService{};
    }
};

} // namespace blog::settings

#ifdef CONGELADO_TEST
namespace blog::settings::tests {
using namespace boost::ut;
suite<"blog_settings_builder"> blog_settings_builder_suite = []
{
    "builds a settings service"_test = []
    {
        const SettingsService service = SettingsServiceBuilder{}.build();
        expect(service.has("title"));
    };
};
} // namespace blog::settings::tests
#endif
