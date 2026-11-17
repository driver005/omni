export module blog_api:http_handler;

import std;
import interfaces;
import :api;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/**
 * @brief Adapts the Ghost API dispatch layer to the HTTP2 server's
 * IRequest/IResponse pair. Plug this handler into core_router to serve the
 * full 261-route API over io/layer/http2.
 */
class BlogHttpHandler
{
public:
    void handle(interfaces::io::IRequest& request, interfaces::io::IResponse& response)
    {
        ApiRequest api_request{
            std::string{request.get_method()},
            std::string{request.get_path()}
        };
        const auto auth = request.get_authorization();
        if (!auth.empty()) {
            api_request.token(std::string{auth});
        }
        const auto bytes = std::ranges::to<std::vector<std::byte>>(request.get_body());
        std::string body;
        body.reserve(bytes.size());
        for (const std::byte byte: bytes) {
            body += static_cast<char>(byte);
        }
        api_request.body(std::move(body));

        const auto result = m_api.dispatch(api_request);
        response.set_status(static_cast<interfaces::io::types::Status>(result.status()));
        std::vector<std::byte> out;
        out.reserve(result.body().size());
        for (const char chr: result.body()) {
            out.push_back(static_cast<std::byte>(chr));
        }
        response.set_body(std::move(out));
    }

    [[nodiscard]] std::size_t route_count() const noexcept
    {
        return m_api.route_count();
    }

private:
    Api m_api{};
};

} // namespace blog::api
