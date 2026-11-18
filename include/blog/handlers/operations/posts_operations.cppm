export module blog_operations:posts_operations;

import std;
import interfaces;
import blog_handlers;
import blog_handlers_runtime;
import blog_handlers_middleware;

export namespace blog::handlers::operations::posts {

template<typename T>
[[nodiscard]] T& special()
{
    return blog::handlers::runtime::ServiceRegistry::special<T>();
}

template<typename T>
[[nodiscard]] T& stripe()
{
    return blog::handlers::runtime::ServiceRegistry::stripe<T>();
}

/** @brief Shared response writer for the self-contained posts operations. */
inline void write(
    interfaces::io::IResponse& response,
    interfaces::io::types::Status status,
    std::string_view body,
    std::function<void()> send
)
{
    std::vector<std::byte> bytes;
    bytes.reserve(body.size());
    for (const char c : body) bytes.push_back(static_cast<std::byte>(c));
    std::move(response).with_status(status).with_body(std::move(bytes));
    send();
}

inline void write_special(
    interfaces::io::IResponse& response,
    blog::api::SpecialResult result,
    std::function<void()> send
)
{
    write(response, static_cast<interfaces::io::types::Status>(result.status()), result.body(), std::move(send));
}

} // namespace blog::handlers::operations::posts
