export module blog_api_core:request;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Concrete API request: method, path, params, body and bearer token. */
class ApiRequest
{
public:
    ApiRequest() = default;

    ApiRequest(std::string method, std::string path) :
        m_method{std::move(method)},
        m_path{std::move(path)}
    {
    }

    [[nodiscard]] const std::string& method() const noexcept { return m_method; }
    [[nodiscard]] const std::string& path() const noexcept { return m_path; }

    ApiRequest& param(std::string key, std::string value)
    {
        m_params.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] const std::map<std::string, std::string>& params() const noexcept
    {
        return m_params;
    }

    [[nodiscard]] std::optional<std::string> param(std::string_view key) const
    {
        const auto it = m_params.find(std::string{key});
        return it == m_params.end() ? std::nullopt : std::optional<std::string>{it->second};
    }

    ApiRequest& body(std::string value) { m_body = std::move(value); return *this; }
    [[nodiscard]] const std::string& body() const noexcept { return m_body; }

    ApiRequest& token(std::string value) { m_token = std::move(value); return *this; }
    [[nodiscard]] const std::string& token() const noexcept { return m_token; }

private:
    std::string m_method{"GET"};
    std::string m_path{"/"};
    std::map<std::string, std::string> m_params{};
    std::string m_body{};
    std::string m_token{};
};

} // namespace blog::api
#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_request"> blog_api_request_suite = []
{
    "carries request data"_test = []
    {
        ApiRequest request{"GET", "/posts/1"};
        request.param("id", "1").body("{}").token("t");
        expect(request.method() == "GET");
        expect(request.param("id") == std::string{"1"});
        expect(request.token() == "t");
    };
};
} // namespace blog::api::tests
#endif
