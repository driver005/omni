export module core_theme:theme;

import std;
import :manifest;
import :package;
import :validator;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::theme {

/** @brief Opaque theme handle. All members inline, shared_ptr pImpl. */
class Theme
{
public:
    Theme() :
        m_runtime{std::make_shared<ThemeRuntime>()}
    {
    }

    Theme(const Theme&) = delete;
    Theme& operator=(const Theme&) = delete;
    Theme(Theme&&) noexcept = default;
    Theme& operator=(Theme&&) noexcept = default;
    ~Theme() = default;

    bool upload(ThemePackage package, std::string& error)
    {
        if (!m_runtime->validator().validate(package, error)) {
            return false;
        }
        m_runtime->store(std::move(package));
        return true;
    }

    [[nodiscard]] const ThemePackage& active() const noexcept
    {
        return m_runtime->package();
    }

    [[nodiscard]] bool has_active() const noexcept
    {
        return m_runtime->package().manifest().valid();
    }

private:
    std::shared_ptr<ThemeRuntime> m_runtime{};
};

} // namespace core::theme

#ifdef CONGELADO_TEST
namespace core::theme::tests {
using namespace boost::ut;

suite<"core_theme_theme"> core_theme_theme_suite = []
{
    "validates and stores a theme"_test = []
    {
        Theme theme;
        std::string error;
        expect(!theme.has_active());
        ThemePackage package{ThemeManifest{"casper", "5.0.0"}};
        package.add_file("package.json", "{}").add_file("default.hbs", "");
        expect(theme.upload(std::move(package), error));
        expect(theme.has_active());
        expect(theme.active().manifest().name() == "casper");
    };
};

} // namespace core::theme::tests
#endif
