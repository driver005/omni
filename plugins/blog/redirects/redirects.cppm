export module blog_redirects:redirects;

import std;
import core_router;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::redirects {

/** @brief Custom-redirect registry backed by core_router redirect rules. */
class RedirectRegistry
{
public:
    void add(std::string from, std::string to, std::uint16_t status = 301)
    {
        m_rules.emplace_back(std::move(from), std::move(to), status);
    }

    [[nodiscard]] std::string target(std::string_view path) const
    {
        for (const auto& rule: m_rules) {
            if (rule.matches(path)) {
                return rule.target(path);
            }
        }
        return {};
    }

private:
    std::vector<core::router::RedirectRule> m_rules{};
};

} // namespace blog::redirects
#ifdef CONGELADO_TEST
namespace blog::redirects::tests {
using namespace boost::ut;
suite<"blog_redirects_redirects"> blog_redirects_redirects_suite = []
{
    "resolves redirects"_test = []
    {
        RedirectRegistry registry;
        registry.add("/old", "/new");
        expect(registry.target("/old/post") == "/new/post");
    };
};
} // namespace blog::redirects::tests
#endif
