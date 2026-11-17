export module core_settings:setting_key;

import std;
import :setting_value;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::settings {

/** @brief A named setting with a typed default and an optional validator tag. */
class SettingKey
{
public:
    SettingKey() = default;

    SettingKey(std::string name, SettingValue default_value, std::string validator_tag = {}) :
        m_name{std::move(name)},
        m_default{std::move(default_value)},
        m_validator_tag{std::move(validator_tag)}
    {
    }

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const SettingValue& default_value() const noexcept
    {
        return m_default;
    }

    [[nodiscard]] const std::string& validator_tag() const noexcept
    {
        return m_validator_tag;
    }

private:
    std::string m_name{};
    SettingValue m_default{};
    std::string m_validator_tag{};
};

} // namespace core::settings

#ifdef CONGELADO_TEST
namespace core::settings::tests {
using namespace boost::ut;

suite<"core_settings_setting_key"> core_settings_setting_key_suite = []
{
    "carries name, default and validator tag"_test = []
    {
        const SettingKey key{"site_title", SettingValue{std::string{"Ghost"}}, "text"};
        expect(key.name() == "site_title");
        expect(std::get<std::string>(key.default_value()) == "Ghost");
        expect(key.validator_tag() == "text");
    };
};

} // namespace core::settings::tests
#endif
