import std;
import blog_handlers_routes;
import blog_handlers;
import core_router;
import io_layer_http2;
import interfaces;
import io_codec_hpack;
import utils_buffering;

/**
 * Wire-level HTTP2 test: real preface, SETTINGS, HPACK-encoded HEADERS and DATA
 * frames flow through ServerFlow into the executor and all 261 routes are
 * installed through RouteRegistrar. No in-process request shortcuts.
 */
int main()
{
    blog::api::BlogHandlers handler;
    core::router::RouterContext<> context;
    blog::api::ApiRoutes{}.install(context, handler);

    io::layer::http2::Server server;
    server.build(&context);

    std::vector<std::byte> sent;
    auto read = server.on_connect(
        [&sent](utils::buffering::BufferNode&& node)
        {
            const auto* data = node.get_data();
            for (std::size_t i = 0; i < node.get_written(); ++i) {
                sent.push_back(data[i]);
            }
        },
        [] {}
    );

    io::layer::http2::HttpRequest outgoing{1};
    outgoing.set_header(interfaces::io::types::Token::METHOD, "GET");
    outgoing.set_header(interfaces::io::types::Token::PATH, "/posts");
    outgoing.set_header(interfaces::io::types::Token::SCHEME, "http");
    outgoing.set_header(interfaces::io::types::Token::AUTHORITY, "localhost");
    outgoing.set_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
    outgoing.set_header(interfaces::io::types::Token::ACCEPT, "application/json");

    io::codec::hpack::HPackTable table;
    auto block = utils::buffering::BufferNode{8'192};
    block | io::layer::http2::WriteHttpRequestAdaptor{outgoing, table, 16'384};

    std::vector<std::byte> wire;
    for (const char chr: std::string_view{"PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"}) {
        wire.push_back(static_cast<std::byte>(chr));
    }
    for (int i = 0; i < 9; ++i) {
        wire.push_back(std::byte{0});
    }
    wire[24 + 3] = static_cast<std::byte>(4); // SETTINGS frame type
    wire.insert(wire.end(), block.get_data(), block.get_data() + block.get_written());

    const auto feed = [&](auto begin, auto end)
    {
        auto* node = new utils::buffering::BufferNode{std::vector<std::byte>{begin, end}};
        utils::buffering::BufferReader reader;
        reader.push_back(node);
        read(reader);
    };
    feed(wire.begin(), wire.begin() + 33);
    for (std::size_t offset = 33; offset + 9 <= wire.size();) {
        const std::size_t length = (std::to_integer<std::size_t>(wire[offset]) << 16) |
                                   (std::to_integer<std::size_t>(wire[offset + 1]) << 8) |
                                   std::to_integer<std::size_t>(wire[offset + 2]);
        const auto end =
            wire.begin() + static_cast<std::ptrdiff_t>(std::min(offset + 9 + length, wire.size()));
        feed(wire.begin() + static_cast<std::ptrdiff_t>(offset), end);
        offset += 9 + length;
    }
    server.executor().on_execute()();

    std::string body;
    for (std::size_t offset = 0; offset + 9 <= sent.size();) {
        const auto at = [&](std::size_t index)
        {
            return std::to_integer<std::size_t>(sent[offset + index]);
        };
        const std::size_t length = (at(0) << 16) | (at(1) << 8) | at(2);
        const std::uint8_t type = static_cast<std::uint8_t>(at(3));
        if (offset + 9 + length > sent.size()) {
            break;
        }
        if (type == 0) {
            for (std::size_t i = 0; i < length; ++i) {
                body += static_cast<char>(sent[offset + 9 + i]);
            }
        }
        offset += 9 + length;
    }

    if (!body.contains("Hello Ghost")) {
        return 1;
    }
    if (!body.contains(R"("meta":{"pagination":)")) {
        return 2;
    }
    if (!body.contains(R"("limit":15)")) {
        return 3;
    }
    return 0;
}
