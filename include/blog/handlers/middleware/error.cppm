export module blog_handlers_middleware:error;

import std;
import interfaces;
import core_serde;

export namespace blog::handlers::middleware {

/** @brief Ghost error envelope seed — reflected so core_serde owns the wire shape. */
struct ErrorItem
{
    std::string id{};
    std::string code{};
    std::string context{};
    std::string help{};
    std::string message{};
    std::string property{};
    std::string type{};
};

struct ErrorEnvelope
{
    std::vector<ErrorItem> errors{};
};

/** @brief Writes an error seed through the IResponse builder, no support helper. */
inline void write_error(
    interfaces::io::IResponse& response,
    interfaces::io::types::Status status,
    const ErrorEnvelope& envelope,
    std::function<void()> send
)
{
    const auto body = core_serde::JsonCodec{}.encode(envelope);
    std::vector<std::byte> bytes;
    bytes.reserve(body.size());
    for (const char c: body) {
        bytes.push_back(static_cast<std::byte>(c));
    }
    std::move(response).with_status(status).with_body(std::move(bytes));
    send();
}

} // namespace blog::handlers::middleware
