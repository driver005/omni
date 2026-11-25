export module core_adapters:tinybird;

import std;
import :config;
import :json;
import :http;
import :outbox_hook;

export namespace core::adapters {

/** @brief Tinybird adapter (token + stats pipe queries). */
class TinybirdAdapter
{
public:
    explicit TinybirdAdapter(
        Config config = {},
        std::shared_ptr<HttpTransport> transport = nullptr
    ) :
        m_config{std::move(config)},
        m_transport{transport ? std::move(transport) : std::make_shared<SocketHttpTransport>()}
    {
    }

    /**
     * @brief Reads/sets the Tinybird token from the configured mock endpoint.
     * Response shape mirrors Ghost: {"tinybird":{"token":...,"exp":...}} or {"tinybird":null}.
     */
    [[nodiscard]] Result token_detailed() const
    {
        std::string path = "/v0/token";
        if (!m_config.workspace_id.empty()) {
            path += std::format("?workspace_id={}", url_encode(m_config.workspace_id));
        }
        return issue("token", "GET", path, {});
    }

    [[nodiscard]] std::string token() const
    {
        return token_detailed().body;
    }

    /** @brief Queries a Tinybird stats pipe; params is a raw query string (e.g. "range=7d"). */
    [[nodiscard]] Result stats_detailed(std::string_view pipe, std::string_view params = {}) const
    {
        std::string path = std::format("/v0/pipes/{}.json", url_encode(pipe));
        if (!params.empty()) {
            path += '?';
            path += params;
        }
        return issue("stats", "GET", path, {});
    }

    [[nodiscard]] std::string stats(std::string_view pipe, std::string_view params = {}) const
    {
        return stats_detailed(pipe, params).body;
    }

    [[nodiscard]] const Config& config() const noexcept
    {
        return m_config;
    }

private:
    [[nodiscard]] Result issue(
        std::string_view operation,
        std::string_view method,
        std::string_view path,
        std::string_view body
    ) const
    {
        if (m_config.base_url.empty()) {
            return {
                503,
                ghost_error(
                    "AdapterNotConfigured",
                    std::format("TINYBIRD_BASE_URL is not set (operation: {})", operation),
                    "config"
                )
            };
        }
        HttpRequest request;
        request.method = method;
        request.url = std::format("{}{}", trim_slash(m_config.base_url), path);
        if (!m_config.api_key.empty()) {
            request.headers.emplace_back(
                "Authorization",
                std::format("Bearer {}", m_config.api_key)
            );
        }
        request.headers.emplace_back("Accept", "application/json");
        request.body = body;

        const auto response = m_transport->send(request);
        record_outbound({
            .adapter = "tinybird",
            .operation = std::string{operation},
            .method = request.method,
            .url = request.url,
            .request_body = request.body,
            .response_body = response.body,
            .error = response.error,
            .status = response.status,
            .ok = response.ok(),
        });
        return normalize(response, "tinybird");
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
