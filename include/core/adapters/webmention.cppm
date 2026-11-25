export module core_adapters:webmention;

import std;
import :config;
import :json;
import :http;
import :outbox_hook;

export namespace core::adapters {

/** @brief Webmention adapter (fetch / source / target) over WEBMENTION_ENDPOINT. */
class WebmentionAdapter
{
public:
    explicit WebmentionAdapter(
        Config config = {},
        std::shared_ptr<HttpTransport> transport = nullptr
    ) :
        m_config{std::move(config)},
        m_transport{transport ? std::move(transport) : std::make_shared<SocketHttpTransport>()}
    {
    }

    /** @brief Fetches the endpoint's discovery/document response for a target URL. */
    [[nodiscard]] Result fetch_detailed(std::string_view target) const
    {
        return issue(
            "fetch",
            "GET",
            std::format("?url={}", url_encode(target)),
            {},
            "application/json"
        );
    }

    [[nodiscard]] std::string fetch(std::string_view target) const
    {
        return fetch_detailed(target).body;
    }

    /** @brief Sends a webmention (source -> target) as application/x-www-form-urlencoded. */
    [[nodiscard]] Result source_detailed(std::string_view source, std::string_view target) const
    {
        const std::string body =
            std::format("source={}&target={}", url_encode(source), url_encode(target));
        return issue("source", "POST", {}, body, "application/x-www-form-urlencoded");
    }

    [[nodiscard]] std::string source(std::string_view source, std::string_view target) const
    {
        return source_detailed(source, target).body;
    }

    /** @brief Reads mentions collected for a target URL. */
    [[nodiscard]] Result target_detailed(std::string_view target) const
    {
        return issue(
            "target",
            "GET",
            std::format("?target={}", url_encode(target)),
            {},
            "application/json"
        );
    }

    [[nodiscard]] std::string target(std::string_view target) const
    {
        return target_detailed(target).body;
    }

    [[nodiscard]] const Config& config() const noexcept
    {
        return m_config;
    }

private:
    [[nodiscard]] Result issue(
        std::string_view operation,
        std::string_view method,
        std::string_view query,
        std::string_view body,
        std::string_view content_type
    ) const
    {
        if (m_config.base_url.empty()) {
            return {
                503,
                ghost_error(
                    "AdapterNotConfigured",
                    std::format("WEBMENTION_ENDPOINT is not set (operation: {})", operation),
                    "config"
                )
            };
        }
        HttpRequest request;
        request.method = method;
        request.url = std::format("{}/webmention{}", trim_slash(m_config.base_url), query);
        if (!m_config.api_key.empty()) {
            request.headers.emplace_back(
                "Authorization",
                std::format("Bearer {}", m_config.api_key)
            );
        }
        request.headers.emplace_back("Content-Type", content_type);
        request.headers.emplace_back("Accept", "application/json");
        request.body = body;

        const auto response = m_transport->send(request);
        record_outbound({
            .adapter = "webmention",
            .operation = std::string{operation},
            .method = request.method,
            .url = request.url,
            .request_body = request.body,
            .response_body = response.body,
            .error = response.error,
            .status = response.status,
            .ok = response.ok(),
        });
        // Webmention endpoints commonly answer 202 with no body; synthesize a Ghost body.
        return normalize(response, "webmention", false);
    }

    [[nodiscard]] static std::string trim_slash(std::string_view url)
    {
        return url.empty() || url.back() != '/' ? std::string{url}
                                                : std::string{url.substr(0, url.size() - 1)};
    }

    Config m_config;
    std::shared_ptr<HttpTransport> m_transport;
};

} // namespace core::adapters
