export module core_theme:validator;

import std;
import :manifest;
import :package;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::theme {

/** @brief Validates a theme package against required files and manifest. */
class ThemeValidator
{
public:
    [[nodiscard]] bool validate(const ThemePackage& package, std::string& error) const
    {
        if (!package.manifest().valid()) {
            error = "manifest missing name or version";
            return false;
        }
        if (!package.has_file("package.json")) {
            error = "package.json missing";
            return false;
        }
        if (!package.has_file("default.hbs") && !package.has_file("index.hbs")) {
            error = "no default.hbs or index.hbs";
            return false;
        }
        error.clear();
        return true;
    }

    [[nodiscard]] std::size_t required_file_count() const noexcept
    {
        return 2;
    }

private:
};

} // namespace core::theme

#ifdef CONGELADO_TEST
namespace core::theme::tests {
using namespace boost::ut;

suite<"core_theme_theme_validator"> core_theme_theme_validator_suite = []
{
    "checks manifest and required files"_test = []
    {
        ThemeValidator validator;
        std::string error;
        ThemePackage invalid{ThemeManifest{"", ""}};
        expect(!validator.validate(invalid, error));
        expect(!error.empty());

        ThemePackage valid{ThemeManifest{"casper", "5.0.0"}};
        valid.add_file("package.json", "{}").add_file("default.hbs", "");
        expect(validator.validate(valid, error));
        expect(error.empty());
        expect(validator.required_file_count() == 2);
    };
};

} // namespace core::theme::tests
#endif
