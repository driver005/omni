export module core_settings:validator;

import std;
import :setting_value;
import :setting_key;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::settings {

/** @brief Validates a SettingValue against a SettingKey's declared type/tag. */
class SettingsValidator
{
public:
    [[nodiscard]] bool accepts(const SettingKey& key, const SettingValue& value) const
    {
        if (std::holds_alternative<std::monostate>(key.default_value())) {
            return !std::holds_alternative<std::monostate>(value);
        }
        if (key.default_value().index() != value.index()) {
            return false;
        }
        return validate_tag(key.validator_tag(), value);
    }

private:
    [[nodiscard]] bool validate_tag(std::string_view tag, const SettingValue& value) const
    {
        if (tag.empty()) {
            return true;
        }
        if (tag == "nonempty") {
            const SettingValueText text;
            return !text(value).empty();
        }
        if (tag == "positive") {
            if (const auto* number = std::get_if<std::int64_t>(&value)) {
                return *number >= 0;
            }
            return false;
        }
        return true;
    }
};

} // namespace core::settings

#ifdef CONGELADO_TEST
namespace core::settings::tests {
using namespace boost::ut;

suite<"core_settings_settings_validator"> core_settings_settings_validator_suite = []
{
    "checks type and tag rules"_test = []
    {
        const SettingsValidator validator;
        const SettingKey text_key{"title", SettingValue{std::string{}}, "nonempty"};
        expect(!validator.accepts(text_key, SettingValue{std::string{}}));
        expect(validator.accepts(text_key, SettingValue{std::string{"x"}}));
        expect(!validator.accepts(text_key, SettingValue{std::int64_t{1}}));

        const SettingKey count_key{"count", SettingValue{std::int64_t{0}}, "positive"};
        expect(validator.accepts(count_key, SettingValue{std::int64_t{5}}));
        expect(!validator.accepts(count_key, SettingValue{std::int64_t{-1}}));
    };
};

} // namespace core::settings::tests
#endif
