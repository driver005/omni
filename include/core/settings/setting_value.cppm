export module core_settings:setting_value;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::settings {

/** @brief Variant value stored in a settings map. */
using SettingValue = std::variant<std::monostate, bool, std::int64_t, double, std::string>;

/** @brief Instance functor converting a SettingValue to text. */
class SettingValueText
{
public:
    [[nodiscard]] std::string operator()(const SettingValue& value) const
    {
        return std::visit(
            [](const auto& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::same_as<T, std::monostate>) {
                    return "";
                } else if constexpr (std::same_as<T, bool>) {
                    return v ? "true" : "false";
                } else if constexpr (std::same_as<T, std::string>) {
                    return v;
                } else {
                    return std::format("{}", v);
                }
            },
            value
        );
    }
};

} // namespace core::settings

#ifdef CONGELADO_TEST
namespace core::settings::tests {
using namespace boost::ut;

suite<"core_settings_setting_value"> core_settings_setting_value_suite = []
{
    "renders values as text"_test = []
    {
        const SettingValueText text;
        expect(text(SettingValue{std::int64_t{42}}) == "42");
        expect(text(SettingValue{true}) == "true");
        expect(text(SettingValue{std::string{"hello"}}) == "hello");
    };
};

} // namespace core::settings::tests
#endif
