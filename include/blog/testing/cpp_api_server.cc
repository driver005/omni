// Minimal HTTP/1.1 front end for the C++ Ghost API. Used by the e2e-api suite
// when it runs against this implementation instead of the Node Ghost server.
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

import std;
import interfaces;
import blog_handlers_routes;
import blog_handlers;
import core_router;
import io_layer_http2;
import utils_buffering;

namespace {

std::string lower(std::string value)
{
    std::ranges::transform(
        value,
        value.begin(),
        [](char c)
        {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    );
    return value;
}

struct Request
{
    std::string method;
    std::string path;
    std::string body;
    std::string authorization;
};

std::optional<Request> read_request(int fd)
{
    std::string raw;
    char buffer[8'192];
    while (raw.find("\r\n\r\n") == std::string::npos) {
        const ssize_t got = recv(fd, buffer, sizeof(buffer), 0);
        if (got <= 0) {
            return std::nullopt;
        }
        raw.append(buffer, static_cast<std::size_t>(got));
    }
    const auto header_end = raw.find("\r\n\r\n");
    const auto head = std::string_view{raw}.substr(0, header_end);
    Request request;
    std::size_t pos = 0;
    for (;;) {
        const auto line_end = head.find("\r\n", pos);
        const auto line = head.substr(
            pos,
            line_end == std::string_view::npos ? std::string_view::npos : line_end - pos
        );
        if (line.empty()) {
            break;
        }
        if (request.method.empty()) {
            const auto space = line.find(' ');
            if (space != std::string_view::npos) {
                request.method = std::string{line.substr(0, space)};
                const auto second = line.find(' ', space + 1);
                request.path = std::string{line.substr(
                    space + 1,
                    second == std::string_view::npos ? std::string_view::npos : second - space - 1
                )};
            }
        } else {
            const auto colon = line.find(':');
            if (colon != std::string_view::npos) {
                const auto name = lower(std::string{line.substr(0, colon)});
                const auto value = std::string{line.substr(colon + 1)};
                if (name == "authorization") {
                    request.authorization = value;
                }
            }
        }
        if (line_end == std::string_view::npos) {
            break;
        }
        pos = line_end + 2;
    }
    std::size_t content_length = request.body.size();
    for (const auto& line: head | std::views::split('\n')) {
        const std::string text{std::ranges::begin(line), std::ranges::end(line)};
        const auto colon = text.find(':');
        if (colon != std::string::npos && lower(text.substr(0, colon)) == "content-length") {
            try {
                content_length = std::stoul(text.substr(colon + 1));
            } catch (...) {
            }
        }
    }
    std::size_t available = raw.size() - header_end - 4;
    while (available < content_length) {
        const ssize_t got = recv(fd, buffer, sizeof(buffer), 0);
        if (got <= 0) {
            break;
        }
        raw.append(buffer, static_cast<std::size_t>(got));
        available += static_cast<std::size_t>(got);
    }
    request.body = raw.substr(header_end + 4, std::min(content_length, available));
    return request;
}

void write_response(int fd, int status, std::string_view body)
{
    const std::string head = std::format(
        "HTTP/1.1 {} OK\r\nContent-Type: application/json; charset=utf-8\r\n"
        "Content-Length: {}\r\nConnection: close\r\n\r\n",
        status,
        body.size()
    );
    std::string out = head + std::string{body};
    std::size_t sent = 0;
    while (sent < out.size()) {
        const ssize_t wrote = send(fd, out.data() + sent, out.size() - sent, 0);
        if (wrote <= 0) {
            break;
        }
        sent += static_cast<std::size_t>(wrote);
    }
}

} // namespace

int main(int argc, char** argv)
{
    const int port = argc > 1 ? std::atoi(argv[1]) : 2'369;
    blog::api::BlogHandlers handlers;
    core::router::RouterContext<> context;
    blog::api::ApiRoutes{}.install(context, handlers);
    io::layer::http2::Server server;
    server.build(&context);

    const int listener = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<std::uint16_t>(port));
    if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        std::println("cpp_api_server: bind {} failed", port);
        return 1;
    }
    listen(listener, 128);
    std::println("cpp_api_server: listening on {}", port);

    for (;;) {
        const int client = accept(listener, nullptr, nullptr);
        if (client < 0) {
            continue;
        }
        const auto request = read_request(client);
        if (!request) {
            close(client);
            continue;
        }
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::METHOD, request->method);
        req.set_header(interfaces::io::types::Token::PATH, request->path);
        req.set_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
        req.set_header(interfaces::io::types::Token::ACCEPT, "application/json");
        if (!request->authorization.empty()) {
            req.set_header(interfaces::io::types::Token::AUTHORIZATION, request->authorization);
        }
        if (!request->body.empty()) {
            std::vector<std::byte> bytes;
            for (const char chr: request->body) {
                bytes.push_back(static_cast<std::byte>(chr));
            }
            req.get_body().push_back(
                new utils::buffering::BufferNode{std::move(bytes)},
                0,
                request->body.size()
            );
        }
        server.executor().enqueue(req, res, [] {});
        server.executor().on_execute()();

        std::string body;
        for (const std::byte byte: res.get_body()) {
            body += static_cast<char>(byte);
        }
        int status = 200;
        for (const auto& entry: res.get_headers()) {
            std::visit(
                [&status](const auto& field)
                {
                    if (!field) {
                        return;
                    }
                    const auto& name = field->get_name();
                    if constexpr (
                        std::is_same_v<std::decay_t<decltype(name)>, interfaces::io::types::Token>
                    ) {
                        if (name == interfaces::io::types::Token::STATUS) {
                            try {
                                status = std::stoi(field->get_value());
                            } catch (...) {
                            }
                        }
                    }
                },
                entry
            );
        }
        write_response(client, status, body);
        close(client);
    }
}
