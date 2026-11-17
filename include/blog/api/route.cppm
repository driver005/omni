export module blog_api:route;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief One Ghost API route: method, path template, resource, action, auth tier. */
class ApiRoute
{
public:
    ApiRoute() = default;

    ApiRoute(std::string method, std::string path, std::string resource, std::string action, bool admin) :
        m_method{std::move(method)},
        m_path{std::move(path)},
        m_resource{std::move(resource)},
        m_action{std::move(action)},
        m_admin{admin}
    {
    }

    [[nodiscard]] const std::string& method() const noexcept { return m_method; }
    [[nodiscard]] const std::string& path() const noexcept { return m_path; }
    [[nodiscard]] const std::string& resource() const noexcept { return m_resource; }
    [[nodiscard]] const std::string& action() const noexcept { return m_action; }
    [[nodiscard]] bool admin() const noexcept { return m_admin; }

    /** @brief Matches a concrete path, extracting :param segments. */
    [[nodiscard]] bool match(std::string_view concrete, std::map<std::string, std::string>& params) const
    {
        auto p = m_path.begin();
        auto c = concrete.begin();
        while (p != m_path.end()) {
            if (*p == ':') {
                auto pend = std::find(p, m_path.end(), '/');
                auto cend = std::find(c, concrete.end(), '/');
                std::string key{p + 1, pend};
                if (key.empty() || c == cend) return false;
                params[key] = std::string{c, cend};
                p = pend; c = cend;
            } else {
                if (c == concrete.end() || *p != *c) return false;
                ++p; ++c;
            }
        }
        return c == concrete.end();
    }

private:
    std::string m_method{"GET"};
    std::string m_path{"/"};
    std::string m_resource{"root"};
    std::string m_action{"get"};
    bool m_admin{false};
};

} // namespace blog::api
#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_route"> blog_api_route_suite = []
{
    "matches param routes"_test = []
    {
        ApiRoute route{"GET", "/posts/:id/posts", "posts", "get", false};
        std::map<std::string, std::string> params;
        expect(route.match("/posts/42/posts", params));
        expect(params["id"] == "42");
    };
};
} // namespace blog::api::tests
#endif
