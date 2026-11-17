export module core_settings:map;

import std;
import :setting_value;
import :setting_key;
import :validator;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::settings {

/** @brief Validated, typed settings storage with registered keys. */
class SettingsMap
{
public:
    bool register_key(SettingKey key)
    {
        const bool inserted = m_keys.contains(key.name());
        m_keys[key.name()] = std::move(key);
        return inserted;
    }

    [[nodiscard]] bool set(std::string_view name, SettingValue value)
    {
        const auto it = m_keys.find(std::string{name});
        if (it == m_keys.end() || !m_validator.accepts(it->second, value)) {
            return false;
        }
        m_values[std::string{name}] = std::move(value);
        return true;
    }

    [[nodiscard]] std::optional<SettingValue> get(std::string_view name) const
    {
        const auto it = m_values.find(std::string{name});
        return it == m_values.end() ? std::nullopt : std::optional<SettingValue>{it->second};
    }

    [[nodiscard]] SettingValue get_or_default(std::string_view name) const
    {
        if (const auto value = get(name)) {
            return *value;
        }
        const auto it = m_keys.find(std::string{name});
        return it == m_keys.end() ? SettingValue{} : it->second.default_value();
    }

    [[nodiscard]] bool contains(std::string_view name) const noexcept
    {
        return m_values.contains(std::string{name});
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_values.size();
    }

    void clear_values() noexcept
    {
        m_values.clear();
    }

private:
    std::map<std::string, SettingKey> m_keys{};
    std::map<std::string, SettingValue> m_values{};
    SettingsValidator m_validator{};
};

} // namespace core::settings

#ifdef CONGELADO_TEST
namespace core::settings::tests {
using namespace boost::ut;

suite<"core_settings_settings_map"> core_settings_settings_map_suite = []
{
    "registers keys and stores validated values"_test = []
    {
        SettingsMap map;
        expect(!map.register_key(SettingKey{"title", SettingValue{std::string{}}, "nonempty"}));
        expect(map.register_key(SettingKey{"title", SettingValue{std::string{}}, "nonempty"}));
        expect(map.set("title", SettingValue{std::string{"Ghost"}}));
        expect(!map.set("title", SettingValue{std::string{}}));
        expect(!map.set("unknown", SettingValue{true}));
        expect(map.contains("title"));
        expect(std::get<std::string>(*map.get("title")) == "Ghost");
        expect(std::get<std::string>(map.get_or_default("title")) == "Ghost");
        expect(map.size() == 1);
    };
};

} // namespace core::settings::tests
#endif
