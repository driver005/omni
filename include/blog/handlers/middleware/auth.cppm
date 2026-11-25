export module blog_handlers_middleware:auth;

import std;
import interfaces;
import :bearer;
import :error;
import :registry;

export namespace blog::handlers::middleware {

/** @brief Rejects requests without a bearer token through the error seed middleware. */
inline void require_auth(
    interfaces::io::IRequest& request,
    interfaces::io::IResponse& response,
    interfaces::NextFn&& next,
    std::function<void()> send
)
{
    if (token(request).empty()) {
        ErrorEnvelope envelope;
        envelope.errors.push_back(
            ErrorItem{.message = "Unauthorized", .type = "UnauthorizedError"}
        );
        write_error(
            response,
            interfaces::io::types::Status::UNAUTHORIZED,
            envelope,
            std::move(send)
        );
        return;
    }
    next(request, response, std::move(send));
}

/** @brief Content resources stay readable without a token, as in Ghost. */
inline void optional_auth(
    interfaces::io::IRequest& request,
    interfaces::io::IResponse& response,
    interfaces::NextFn&& next,
    std::function<void()> send
)
{
    next(request, response, std::move(send));
}

} // namespace blog::handlers::middleware
