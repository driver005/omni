export module core_theme:builder;

import std;
import :theme;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::theme {

/** @brief Fluent entry point — sole public way to construct a Theme handle. */
class ThemeBuilder
{
public:
    ThemeBuilder& option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Theme build() const
    {
        return Theme{};
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace core::theme

#ifdef CONGELADO_TEST
namespace core::theme::tests {
using namespace boost::ut;

suite<"core_theme_theme_builder"> core_theme_theme_builder_suite = []
{
    "builds a Theme handle"_test = []
    {
        const Theme theme = ThemeBuilder{}.option("dir", "content/themes").build();
        expect(!theme.has_active());
    };
};

} // namespace core::theme::tests
#endif
