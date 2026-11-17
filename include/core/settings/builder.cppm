export module core_settings:builder;

import std;
import :settings;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::settings {

/** @brief Fluent entry point — sole public way to construct a Settings handle. */
class SettingsBuilder
{
public:
    SettingsBuilder& option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Settings build() const
    {
        return Settings{};
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace core::settings

#ifdef CONGELADO_TEST
namespace core::settings::tests {
using namespace boost::ut;

suite<"core_settings_settings_builder"> core_settings_settings_builder_suite = []
{
    "builds a working Settings handle"_test = []
    {
        const Settings settings = SettingsBuilder{}.option("env", "test").build();
        expect(settings.size() == 0);
        expect(!settings.contains("env"));
    };
};

} // namespace core::settings::tests
#endif
