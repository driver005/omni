export module blog_handlers_middleware:bearer;

import std;
import interfaces;

export namespace blog::handlers::middleware {

inline constexpr std::string_view kTokenHeader{"x-blog-token"};

/** @brief Extracts a bearer/admin token and normalizes it into a request header. */
inline void bearer_middleware(
    interfaces::io::IRequest& request,
    interfaces::io::IResponse& response,
    interfaces::NextFn&& next,
    std::function<void()> send
)
{
    auto authorization = std::string{request.get_authorization()};
    if (authorization.starts_with("Bearer ")) {
        authorization.erase(0, std::string{"Bearer "}.size());
    }
    request.set_header(std::string_view{kTokenHeader}, authorization);
    next(request, response, std::move(send));
}

/** @brief Reads the normalized token left by bearer_middleware. */
[[nodiscard]] inline std::string_view token(interfaces::io::IRequest& request)
{
    const auto authorization = request.get_authorization();
    return authorization.starts_with("Bearer ")
               ? authorization.substr(std::string_view{"Bearer "}.size())
               : authorization;
}

} // namespace blog::handlers::middleware
