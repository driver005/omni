module;

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

export module core_adapters:http;

import std;
import :json;

export namespace core::adapters {

/** @brief Parsed absolute URL. Only http(s) is supported; mocks use plain http on loopback. */
struct ParsedUrl
{
    std::string scheme{};
    std::string host{};
    std::string authority{}; // host[:port], for the Host header
    std::string path{};      // always begins with '/'
    std::string query{};     // without '?'
    std::uint16_t port{80};
};

[[nodiscard]] inline std::optional<ParsedUrl> parse_url(std::string_view url)
{
    const auto scheme_end = url.find("://");
    if (scheme_end == std::string_view::npos) {
        return std::nullopt;
    }
    ParsedUrl parsed;
    parsed.scheme = std::string{url.substr(0, scheme_end)};
    if (parsed.scheme != "http" && parsed.scheme != "https") {
        return std::nullopt;
    }
    parsed.port = parsed.scheme == "https" ? 443 : 80;

    const auto rest = url.substr(scheme_end + 3);
    const auto authority_end = rest.find_first_of("/?");
    const auto authority = rest.substr(0, authority_end);
    if (authority.empty()) {
        return std::nullopt;
    }
    // Optional userinfo is not used by adapter configs.
    const auto at = authority.find('@');
    const auto hostport = at == std::string_view::npos ? authority : authority.substr(at + 1);
    const auto colon = hostport.rfind(':');
    if (colon != std::string_view::npos && colon + 1 < hostport.size()) {
        const auto port_text = hostport.substr(colon + 1);
        std::uint16_t parsed_port = 0;
        const auto result =
            std::from_chars(port_text.data(), port_text.data() + port_text.size(), parsed_port);
        if (result.ec != std::errc{} || parsed_port == 0) {
            return std::nullopt;
        }
        parsed.port = parsed_port;
        parsed.host = std::string{hostport.substr(0, colon)};
    } else {
        parsed.host = std::string{hostport};
    }
    parsed.authority = parsed.host == std::string{hostport}
                           ? parsed.host
                           : std::format("{}:{}", parsed.host, parsed.port);
    if (parsed.host.empty()) {
        return std::nullopt;
    }

    if (authority_end == std::string_view::npos) {
        parsed.path = "/";
        return parsed;
    }
    const auto tail = rest.substr(authority_end);
    if (tail.front() == '?') {
        parsed.path = "/";
        parsed.query = std::string{tail.substr(1)};
    } else {
        const auto question = tail.find('?');
        parsed.path = question == std::string_view::npos ? std::string{tail}
                                                         : std::string{tail.substr(0, question)};
        if (question != std::string_view::npos) {
            parsed.query = std::string{tail.substr(question + 1)};
        }
    }
    return parsed;
}

/** @brief Synchronous outbound HTTP request. */
struct HttpRequest
{
    std::string method{"GET"};
    std::string url{};
    std::vector<std::pair<std::string, std::string>> headers{};
    std::string body{};
};

/** @brief Synchronous outbound HTTP response. Transport errors never throw. */
struct HttpResponse
{
    int status{0};
    std::string body{};
    std::vector<std::pair<std::string, std::string>> headers{};
    std::string error{};

    [[nodiscard]] bool ok() const noexcept
    {
        return error.empty() && status >= 200 && status < 300;
    }
};

/** @brief Adapter result: HTTP status plus a Ghost-shaped JSON body. */
struct Result
{
    int status{200};
    std::string body{};

    [[nodiscard]] bool ok() const noexcept
    {
        return status >= 200 && status < 300;
    }
};

/** @brief Injectable transport so adapters can be verified without live credentials. */
class HttpTransport
{
public:
    virtual ~HttpTransport() = default;
    [[nodiscard]] virtual HttpResponse send(const HttpRequest& request) = 0;
};

/**
 * @brief Real HTTP/1.1 client over a POSIX socket.
 *
 * Targets the local mock base URLs from STRIPE_BASE_URL / TINYBIRD_BASE_URL / SMTP_URL /
 * WEBMENTION_ENDPOINT. TLS is intentionally rejected here: hosts that own an HTTP/2
 * runtime should inject CoreClientTransport instead.
 */
class SocketHttpTransport final : public HttpTransport
{
public:
    [[nodiscard]] HttpResponse send(const HttpRequest& request) override
    {
        HttpResponse response;
        const auto parsed = parse_url(request.url);
        if (!parsed) {
            response.error = std::format("invalid or unsupported URL: {}", request.url);
            return response;
        }
        if (parsed->scheme == "https") {
            response.error = "https requires a CoreClientTransport (TLS runtime not owned here)";
            return response;
        }
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
        addrinfo hints{};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        addrinfo* resolved = nullptr;
        const auto lookup = ::getaddrinfo(
            parsed->host.c_str(),
            std::to_string(parsed->port).c_str(),
            &hints,
            &resolved
        );
        if (lookup != 0 || resolved == nullptr) {
            response.error = std::format("DNS lookup failed for {}", parsed->host);
            if (resolved != nullptr) {
                ::freeaddrinfo(resolved);
            }
            return response;
        }

        int socket_fd = -1;
        for (auto* candidate = resolved; candidate != nullptr; candidate = candidate->ai_next) {
            socket_fd =
                ::socket(candidate->ai_family, candidate->ai_socktype, candidate->ai_protocol);
            if (socket_fd < 0) {
                continue;
            }
            timeval timeout{};
            timeout.tv_sec = 5;
            ::setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            ::setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
            if (::connect(socket_fd, candidate->ai_addr, candidate->ai_addrlen) == 0) {
                break;
            }
            ::close(socket_fd);
            socket_fd = -1;
        }
        ::freeaddrinfo(resolved);
        if (socket_fd < 0) {
            response.error = std::format("connect failed for {}", parsed->authority);
            return response;
        }

        std::string path_with_query = parsed->path;
        if (!parsed->query.empty()) {
            path_with_query += '?';
            path_with_query += parsed->query;
        }
        std::string wire = std::format(
            "{} {} HTTP/1.1\r\nHost: {}\r\n",
            request.method,
            path_with_query,
            parsed->authority
        );
        for (const auto& [name, value]: request.headers) {
            wire += std::format("{}: {}\r\n", name, value);
        }
        wire += std::format("Content-Length: {}\r\nConnection: close\r\n\r\n", request.body.size());
        wire += request.body;

        std::size_t sent = 0;
        while (sent < wire.size()) {
            const auto count = ::send(socket_fd, wire.data() + sent, wire.size() - sent, 0);
            if (count <= 0) {
                response.error = "socket send failed";
                ::close(socket_fd);
                return response;
            }
            sent += static_cast<std::size_t>(count);
        }

        std::string received;
        char buffer[4'096];
        std::size_t expected_body = std::string::npos;
        while (true) {
            const auto count = ::recv(socket_fd, buffer, sizeof(buffer), 0);
            if (count < 0) {
                response.error = "socket recv failed";
                break;
            }
            if (count == 0) {
                break;
            }
            received.append(buffer, static_cast<std::size_t>(count));
            const auto header_end = received.find("\r\n\r\n");
            if (header_end != std::string::npos && expected_body == std::string::npos) {
                const auto headers_text = received.substr(0, header_end);
                for (const auto& line: split_lines(headers_text)) {
                    const auto colon = line.find(':');
                    if (colon == std::string::npos) {
                        continue;
                    }
                    std::string name = line.substr(0, colon);
                    std::string value = line.substr(colon + 1);
                    trim(name);
                    trim(value);
                    response.headers.emplace_back(std::move(name), std::move(value));
                    std::string lower = name;
                    std::ranges::transform(
                        lower,
                        lower.begin(),
                        [](const char c)
                        {
                            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                        }
                    );
                    if (lower == "content-length") {
                        std::size_t length = 0;
                        const auto parsed_length =
                            std::from_chars(value.data(), value.data() + value.size(), length);
                        if (parsed_length.ec == std::errc{}) {
                            expected_body = length;
                        }
                    }
                }
            }
            if (header_end != std::string::npos && expected_body != std::string::npos &&
                received.size() >= header_end + 4 + expected_body) {
                break;
            }
        }
        ::close(socket_fd);

        const auto header_end = received.find("\r\n\r\n");
        if (header_end == std::string::npos) {
            if (response.error.empty()) {
                response.error = "malformed HTTP response (no header terminator)";
            }
            return response;
        }
        const auto first_line_end = received.find("\r\n");
        if (first_line_end == std::string::npos || first_line_end > header_end) {
            response.error = "malformed HTTP status line";
            return response;
        }
        const auto status_line = received.substr(0, first_line_end);
        const auto space = status_line.find(' ');
        if (space == std::string::npos) {
            response.error = "malformed HTTP status line";
            return response;
        }
        const auto status_text = status_line.substr(space + 1);
        const auto parse_result = std::from_chars(
            status_text.data(),
            status_text.data() + status_text.size(),
            response.status
        );
        if (parse_result.ec != std::errc{}) {
            response.error = "malformed HTTP status code";
            return response;
        }
        response.body = received.substr(header_end + 4);
        if (expected_body != std::string::npos && response.body.size() > expected_body) {
            response.body.resize(expected_body);
        }
#else
        response.error = "socket transport unsupported on this platform";
#endif
        return response;
    }

private:
    [[nodiscard]] static std::vector<std::string> split_lines(std::string_view text)
    {
        std::vector<std::string> lines;
        std::size_t start = 0;
        while (start < text.size()) {
            const auto end = text.find('\n', start);
            if (end == std::string_view::npos) {
                lines.emplace_back(text.substr(start));
                break;
            }
            auto line = std::string{text.substr(start, end - start)};
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(std::move(line));
            start = end + 1;
        }
        return lines;
    }

    static void trim(std::string& text)
    {
        const auto first = text.find_first_not_of(" \t");
        if (first == std::string::npos) {
            text.clear();
            return;
        }
        const auto last = text.find_last_not_of(" \t");
        text = text.substr(first, last - first + 1);
    }
};

/** @brief In-process mock transport for unit tests and local verification. */
class MockHttpTransport final : public HttpTransport
{
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    /** @brief Routes by HTTP method and a substring of the full request URL. */
    void route(std::string method, std::string url_contains, Handler handler)
    {
        m_routes.push_back({std::move(method), std::move(url_contains), std::move(handler)});
    }

    void set_default(Handler handler)
    {
        m_default = std::move(handler);
    }

    [[nodiscard]] HttpResponse send(const HttpRequest& request) override
    {
        m_requests.push_back(request);
        for (const auto& entry: m_routes) {
            if (entry.method == request.method && request.url.contains(entry.url_contains)) {
                return entry.handler(request);
            }
        }
        if (m_default) {
            return m_default(request);
        }
        HttpResponse response;
        response.status = 404;
        response.body = ghost_error(
            "NotFound",
            std::format("no mock route for {} {}", request.method, request.url)
        );
        return response;
    }

    [[nodiscard]] const std::vector<HttpRequest>& requests() const noexcept
    {
        return m_requests;
    }

private:
    struct Route
    {
        std::string method;
        std::string url_contains;
        Handler handler;
    };

    std::vector<Route> m_routes{};
    std::vector<HttpRequest> m_requests{};
    Handler m_default{};
};

/**
 * @brief Turns a transport response into a Ghost-shaped body.
 *
 * Success bodies are passed through verbatim (the upstream mock already returns a Ghost
 * envelope). Failures return documented Ghost error envelopes instead of throwing.
 */
[[nodiscard]] inline Result
normalize(const HttpResponse& response, std::string_view provider, bool require_json = true)
{
    if (!response.error.empty()) {
        return {
            502,
            ghost_error(
                "AdapterTransportError",
                std::format("{} outbound call failed: {}", provider, response.error),
                "transport"
            )
        };
    }
    const int status = response.status == 0 ? 502 : response.status;
    if (status < 200 || status >= 300) {
        return {
            status,
            ghost_error(
                "UpstreamError",
                std::format("{} upstream returned status {}", provider, status),
                std::format("http_{}", status),
                response.body
            )
        };
    }
    if (response.body.empty()) {
        if (!require_json) {
            return {status, std::format(R"({{"{}":{{"status":{}}}}})", provider, status)};
        }
        return {
            502,
            ghost_error("AdapterEmptyResponse", std::format("{} returned an empty body", provider))
        };
    }
    if (require_json && !valid_json(response.body)) {
        return {
            502,
            ghost_error(
                "AdapterInvalidJson",
                std::format("{} returned a non-JSON body", provider),
                "parse",
                response.body
            )
        };
    }
    return {status, response.body};
}

} // namespace core::adapters
