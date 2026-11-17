export module core_settings:settings;

import std;
import :setting_key;
import :setting_value;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::settings {

/** @brief Opaque settings handle. All members inline, shared_ptr pImpl. */
class Settings
{
public:
    Settings() :
        m_runtime{std::make_shared<SettingsRuntime>()}
    {
    }

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings(Settings&&) noexcept = default;
    Settings& operator=(Settings&&) noexcept = default;
    ~Settings() = default;

    bool register_key(SettingKey key)
    {
        return m_runtime->map().register_key(std::move(key));
    }

    bool set(std::string_view name, SettingValue value)
    {
        return m_runtime->map().set(name, std::move(value));
    }

    [[nodiscard]] std::optional<SettingValue> get(std::string_view name) const
    {
        return m_runtime->map().get(name);
    }

    [[nodiscard]] SettingValue get_or_default(std::string_view name) const
    {
        return m_runtime->map().get_or_default(name);
    }

    [[nodiscard]] bool contains(std::string_view name) const noexcept
    {
        return m_runtime->map().contains(name);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_runtime->map().size();
    }

    void clear_values() noexcept
    {
        m_runtime->map().clear_values();
    }

private:
    std::shared_ptr<SettingsRuntime> m_runtime{};
};

} // namespace core::settings

#ifdef CONGELADO_TEST
namespace core::settings::tests {
using namespace boost::ut;

suite<"core_settings_settings"> core_settings_settings_suite = []
{
    "facade stores and reads settings"_test = []
    {
        Settings settings;
        settings.register_key(SettingKey{"title", SettingValue{std::string{}}, "nonempty"});
        expect(settings.set("title", SettingValue{std::string{"Ghost"}}));
        expect(std::get<std::string>(*settings.get("title")) == "Ghost");
        expect(settings.size() == 1);
        settings.clear_values();
        expect(settings.size() == 0);
    };
};

} // namespace core::settings::tests
#endif
