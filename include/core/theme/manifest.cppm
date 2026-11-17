export module core_theme:manifest;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::theme {

/** @brief Parsed theme manifest (package.json subset). */
class ThemeManifest
{
public:
    ThemeManifest() = default;

    ThemeManifest(std::string name, std::string version) :
        m_name{std::move(name)},
        m_version{std::move(version)}
    {
    }

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& version() const noexcept
    {
        return m_version;
    }

    ThemeManifest& add_engine(std::string name, std::string range)
    {
        m_engines.insert_or_assign(std::move(name), std::move(range));
        return *this;
    }

    [[nodiscard]] std::optional<std::string> engine(std::string_view name) const
    {
        const auto it = m_engines.find(std::string{name});
        return it == m_engines.end() ? std::nullopt : std::optional<std::string>{it->second};
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return !m_name.empty() && !m_version.empty();
    }

private:
    std::string m_name{};
    std::string m_version{};
    std::map<std::string, std::string> m_engines{};
};

} // namespace core::theme

#ifdef CONGELADO_TEST
namespace core::theme::tests {
using namespace boost::ut;

suite<"core_theme_theme_manifest"> core_theme_theme_manifest_suite = []
{
    "carries theme metadata"_test = []
    {
        ThemeManifest manifest{"casper", "5.0.0"};
        manifest.add_engine("ghost", ">=5.0.0");
        expect(manifest.valid());
        expect(manifest.name() == "casper");
        expect(manifest.engine("ghost") == std::string{">=5.0.0"});
    };
};

} // namespace core::theme::tests
#endif
