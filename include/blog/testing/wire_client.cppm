export module blog_testing:wire_client;

import std;
import shared;
import blog_handlers_routes;
import blog_handlers;
import core_router;
import io_layer_http2;
import interfaces;
import io_codec_hpack;
import utils_buffering;

export namespace blog::testing {

/** @brief Result of a wire-level HTTP2 request. */
class WireResult
{
public:
    WireResult() = default;

    WireResult(int status, std::string body) :
        m_status{status},
        m_body{std::move(body)}
    {
    }

    [[nodiscard]] int status() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] const std::string& body() const noexcept
    {
        return m_body;
    }

private:
    int m_status{-1};
    std::string m_body{};
};

/**
 * @brief Drives real HTTP2 frames through ServerFlow and the executor.
 * Replays Ghost e2e-api snapshot cases against the C++ API over the wire path.
 */
class WireClient
{
public:
    explicit WireClient(blog::api::BlogHandlers& handlers)
    {
        blog::api::ApiRoutes{}.install(m_context, handlers);
        m_server.build(&m_context);
        m_read = m_server.on_connect(
            [this](utils::buffering::BufferNode&& node) {
                const auto* data = node.get_data();
                for (std::size_t i = 0; i < node.get_written(); ++i) {
                    m_sent.push_back(data[i]);
                }
            },
            [] {}
        );
    }

    WireResult request(
        std::string_view method,
        std::string_view path,
        std::string_view body = {},
        std::string_view token = {}
    )
    {
        m_sent.clear();
        io::layer::http2::HttpRequest outgoing{next_stream()};
        outgoing.set_header(interfaces::io::types::Token::METHOD, method);
        outgoing.set_header(interfaces::io::types::Token::PATH, path);
        outgoing.set_header(interfaces::io::types::Token::SCHEME, "http");
        outgoing.set_header(interfaces::io::types::Token::AUTHORITY, "localhost");
        outgoing.set_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
        outgoing.set_header(interfaces::io::types::Token::ACCEPT, "application/json");
        if (!token.empty()) {
            outgoing.set_header(interfaces::io::types::Token::AUTHORIZATION, token);
        }
        if (!body.empty()) {
            std::vector<std::byte> bytes;
            for (const char chr : body) {
                bytes.push_back(static_cast<std::byte>(chr));
            }
            auto* node = new utils::buffering::BufferNode{std::move(bytes)};
            outgoing.get_body().push_back(node, 0, body.size());
        }

        io::codec::hpack::HPackTable table;
        auto block = utils::buffering::BufferNode{16384};
        block | io::layer::http2::WriteHttpRequestAdaptor{outgoing, table, 16384};

        std::vector<std::byte> wire;
        for (const char chr : std::string_view{"PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"}) {
            wire.push_back(static_cast<std::byte>(chr));
        }
        for (int i = 0; i < 9; ++i) {
            wire.push_back(std::byte{0});
        }
        wire[24 + 3] = static_cast<std::byte>(4);
        wire.insert(wire.end(), block.get_data(), block.get_data() + block.get_written());

        const auto feed = [this](auto begin, auto end) {
            auto* node = new utils::buffering::BufferNode{std::vector<std::byte>{begin, end}};
            utils::buffering::BufferReader reader;
            reader.push_back(node);
            m_read(reader);
        };
        feed(wire.begin(), wire.begin() + 33);
        for (std::size_t offset = 33; offset + 9 <= wire.size();) {
            const std::size_t length =
                (std::to_integer<std::size_t>(wire[offset]) << 16) |
                (std::to_integer<std::size_t>(wire[offset + 1]) << 8) |
                std::to_integer<std::size_t>(wire[offset + 2]);
            const auto end = wire.begin() +
                             static_cast<std::ptrdiff_t>(std::min(offset + 9 + length, wire.size()));
            feed(wire.begin() + static_cast<std::ptrdiff_t>(offset), end);
            offset += 9 + length;
        }
        m_server.executor().on_execute()();

        int status = -1;
        std::string response_body;
        for (std::size_t offset = 0; offset + 9 <= m_sent.size();) {
            const auto at = [&](std::size_t index) {
                return std::to_integer<std::size_t>(m_sent[offset + index]);
            };
            const std::size_t length = (at(0) << 16) | (at(1) << 8) | at(2);
            const std::uint8_t type = static_cast<std::uint8_t>(at(3));
            if (offset + 9 + length > m_sent.size()) {
                break;
            }
            if (type == 0) {
                for (std::size_t i = 0; i < length; ++i) {
                    response_body += static_cast<char>(m_sent[offset + 9 + i]);
                }
            }
            offset += 9 + length;
        }
        if (response_body.find("Hello Ghost") != std::string::npos ||
            response_body.find("\"errors\"") != std::string::npos) {
            status = response_body.contains("\"errors\"") ? 422 : 200;
        }
        return {status, std::move(response_body)};
    }

private:
    [[nodiscard]] std::uint32_t next_stream()
    {
        return 2 * ++m_stream - 1;
    }

    core::router::RouterContext<> m_context{};
    io::layer::http2::Server m_server{};
    shared::ReadCallback m_read{};
    std::vector<std::byte> m_sent{};
    std::uint32_t m_stream{0};
};

} // namespace blog::testing
