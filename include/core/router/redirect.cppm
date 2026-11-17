export module core_router:redirect;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::router {

/** @brief A redirect rule with a status code and literal prefix matching. */
class RedirectRule
{
public:
    RedirectRule() = default;

    RedirectRule(std::string from, std::string to, std::uint16_t status = 301) :
        m_from{std::move(from)},
        m_to{std::move(to)},
        m_status{status}
    {
    }

    [[nodiscard]] bool matches(std::string_view path) const
    {
        return !m_from.empty() && path.starts_with(m_from);
    }

    [[nodiscard]] std::string target(std::string_view path) const
    {
        if (!matches(path)) {
            return {};
        }
        return m_to + std::string{path.substr(m_from.size())};
    }

    [[nodiscard]] std::uint16_t status() const noexcept
    {
        return m_status;
    }

private:
    std::string m_from{};
    std::string m_to{};
    std::uint16_t m_status{301};
};

} // namespace core::router

#ifdef CONGELADO_TEST
namespace core::router::tests {
using namespace boost::ut;

suite<"core_router_redirect"> core_router_redirect_suite = []
{
    "matches prefix and builds target"_test = []
    {
        const RedirectRule rule{"/old", "/new", 302};
        expect(rule.matches("/old/post"));
        expect(!rule.matches("/other"));
        expect(rule.target("/old/post") == "/new/post");
        expect(rule.status() == 302);
    };
};

} // namespace core::router::tests
#endif
