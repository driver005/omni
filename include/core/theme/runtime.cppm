export module core_theme:runtime;

import std;
import :manifest;
import :package;
import :validator;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::theme {
class ThemeRuntime;
}

namespace core::theme {

/** @brief Internal state owned by the facade. */
class ThemeRuntime
{
public:
    void store(ThemePackage package)
    {
        m_package = std::move(package);
    }

    [[nodiscard]] const ThemePackage& package() const noexcept
    {
        return m_package;
    }

    [[nodiscard]] ThemeValidator& validator() noexcept
    {
        return m_validator;
    }

private:
    ThemePackage m_package{};
    ThemeValidator m_validator{};
};

} // namespace core::theme

#ifdef CONGELADO_TEST
namespace core::theme::tests {
using namespace boost::ut;

suite<"core_theme_runtime"> core_theme_runtime_suite = []
{
    "stores a theme package"_test = []
    {
        ThemeRuntime runtime;
        runtime.store(ThemePackage{ThemeManifest{"casper", "5.0.0"}});
        expect(runtime.package().manifest().name() == "casper");
    };
};

} // namespace core::theme::tests
#endif
