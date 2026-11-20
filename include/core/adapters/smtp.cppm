export module core_adapters:smtp;

import std;
import :config;
import :json;
import :http;
import :outbox_hook;

export namespace core::adapters {

/**
 * @brief SMTP adapter.
 *
 * SMTP_URL points at an HTTP mail gateway (local mock in tests). The message is POSTed as
 * JSON with the configured From address. Real SMTP-protocol delivery would need an SMTP
 * transport injected in place of HttpTransport; the call-recording path is unchanged.
 */
class SmtpAdapter
{
public:
    explicit SmtpAdapter(
        Config config = {},
        std::shared_ptr<HttpTransport> transport = nullptr
    ) :
        m_config{std::move(config)},
        m_transport{transport ? std::move(transport) : std::make_shared<SocketHttpTransport>()}
    {
    }

    /** @brief Sends a raw message. Never throws; false on any failure. */
    bool send(std::string_view message)
    {
        m_last = issue(message);
        return m_last.ok();
    }

    /** @brief Detailed result for callers that want the Ghost-shaped error body. */
    [[nodiscard]] const Result& send_detailed(std::string_view message)
    {
        m_last = issue(message);
        return m_last;
    }

    [[nodiscard]] std::string last_error_json() const
    {
        return m_last.ok() ? std::string{} : m_last.body;
    }

    [[nodiscard]] const Config& config() const noexcept
    {
        return m_config;
    }

private:
    [[nodiscard]] Result issue(std::string_view message) const
    {
        if (m_config.base_url.empty()) {
            return {
                503,
                ghost_error("AdapterNotConfigured", "SMTP_URL is not set", "config")
            };
        }
        HttpRequest request;
        request.method = "POST";
        request.url = std::format("{}/send", trim_slash(m_config.base_url));
        request.headers.emplace_back("Content-Type", "application/json");
        if (!m_config.api_key.empty()) {
            request.headers.emplace_back("Authorization", std::format("Bearer {}", m_config.api_key));
        }
        request.body = std::format(
            R"({{"from":{},"message":{}}})",
            quote(m_config.from_address),
            quote(message)
        );

        const auto response = m_transport->send(request);
        record_outbound({
            .adapter = "smtp",
            .operation = "send",
            .method = request.method,
            .url = request.url,
            .request_body = request.body,
            .response_body = response.body,
            .error = response.error,
            .status = response.status,
            .ok = response.ok(),
        });
        // A mail gateway may answer 202 with an empty body — that is a success.
        return normalize(response, "smtp", false);
    }

    [[nodiscard]] static std::string trim_slash(std::string_view url)
    {
        return url.empty() || url.back() != '/'
            ? std::string{url}
            : std::string{url.substr(0, url.size() - 1)};
    }

    Config m_config;
    std::shared_ptr<HttpTransport> m_transport;
    Result m_last{};
};

} // namespace core::adapters
