export module core_theme:package;

import std;
import :manifest;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::theme {

/** @brief An uploaded theme: manifest plus named file contents. */
class ThemePackage
{
public:
    ThemePackage() = default;

    explicit ThemePackage(ThemeManifest manifest) :
        m_manifest{std::move(manifest)}
    {
    }

    [[nodiscard]] const ThemeManifest& manifest() const noexcept
    {
        return m_manifest;
    }

    ThemePackage& add_file(std::string name, std::string content)
    {
        m_files.insert_or_assign(std::move(name), std::move(content));
        return *this;
    }

    [[nodiscard]] std::optional<std::string> file(std::string_view name) const
    {
        const auto it = m_files.find(std::string{name});
        return it == m_files.end() ? std::nullopt : std::optional<std::string>{it->second};
    }

    [[nodiscard]] bool has_file(std::string_view name) const noexcept
    {
        return m_files.contains(std::string{name});
    }

    [[nodiscard]] std::size_t file_count() const noexcept
    {
        return m_files.size();
    }

private:
    ThemeManifest m_manifest{};
    std::map<std::string, std::string> m_files{};
};

} // namespace core::theme

#ifdef CONGELADO_TEST
namespace core::theme::tests {
using namespace boost::ut;

suite<"core_theme_theme_package"> core_theme_theme_package_suite = []
{
    "holds manifest and files"_test = []
    {
        ThemePackage package{ThemeManifest{"casper", "5.0.0"}};
        package.add_file("default.hbs", "<h1>{{title}}</h1>");
        expect(package.has_file("default.hbs"));
        expect(package.file("default.hbs") == std::string{"<h1>{{title}}</h1>"});
        expect(package.file_count() == 1);
    };
};

} // namespace core::theme::tests
#endif
