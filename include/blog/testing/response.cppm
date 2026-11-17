export module blog_testing:response;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Concrete API response: status code, JSON body and error flag. */
class ApiResponse
{
public:
    ApiResponse() = default;

    ApiResponse(int status, std::string body, bool error = false) :
        m_status{status},
        m_body{std::move(body)},
        m_error{error}
    {
    }

    [[nodiscard]] int status() const noexcept { return m_status; }
    [[nodiscard]] const std::string& body() const noexcept { return m_body; }
    [[nodiscard]] bool error() const noexcept { return m_error; }
    [[nodiscard]] bool ok() const noexcept { return !m_error && m_status >= 200 && m_status < 300; }

private:
    int m_status{200};
    std::string m_body{"{}"};
    bool m_error{false};
};

} // namespace blog::api
#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_response"> blog_api_response_suite = []
{
    "reports status"_test = []
    {
        expect(ApiResponse{200, "{}"}.ok());
        expect(!ApiResponse{401, "{}", true}.ok());
    };
};
} // namespace blog::api::tests
#endif
