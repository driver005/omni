export module core_adapters:stripe;

import std;
import :config;
import :json;
import :http;
import :outbox_hook;

export namespace core::adapters {

/** @brief Stripe Checkout adapter (create_session / configure / branding / preview). */
class StripeAdapter
{
public:
    explicit StripeAdapter(
        Config config = {},
        std::shared_ptr<HttpTransport> transport = nullptr
    ) :
        m_config{std::move(config)},
        m_transport{transport ? std::move(transport) : std::make_shared<SocketHttpTransport>()}
    {
    }

    [[nodiscard]] Result create_session_detailed(std::string_view session_json) const
    {
        return issue(
            "create_session",
            "POST",
            "/v1/checkout/sessions",
            session_json.empty() ? std::string{"{}"} : std::string{session_json}
        );
    }
    [[nodiscard]] std::string create_session(std::string_view session_json) const
    {
        return create_session_detailed(session_json).body;
    }

    [[nodiscard]] Result configure_detailed(std::string_view config_json) const
    {
        return issue(
            "configure",
            "PUT",
            "/v1/checkout/config",
            config_json.empty() ? std::string{"{}"} : std::string{config_json}
        );
    }
    [[nodiscard]] std::string configure(std::string_view config_json) const
    {
        return configure_detailed(config_json).body;
    }

    [[nodiscard]] Result branding_detailed() const
    {
        return issue("branding", "GET", "/v1/checkout/branding", {});
    }
    [[nodiscard]] std::string branding() const
    {
        return branding_detailed().body;
    }

    [[nodiscard]] Result preview_detailed(std::string_view preview_json) const
    {
        return issue(
            "preview",
            "POST",
            "/v1/checkout/preview",
            preview_json.empty() ? std::string{"{}"} : std::string{preview_json}
        );
    }
    [[nodiscard]] std::string preview(std::string_view preview_json) const
    {
        return preview_detailed(preview_json).body;
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
                    std::format("STRIPE_BASE_URL is not set (operation: {})", operation),
                    "config"
                )
            };
        }
        HttpRequest request;
        request.method = method;
        request.url = std::format("{}{}", trim_slash(m_config.base_url), path);
        if (!m_config.api_key.empty()) {
            request.headers.emplace_back("Authorization", std::format("Bearer {}", m_config.api_key));
        }
        request.headers.emplace_back("Content-Type", "application/json");
        request.body = body;

        const auto response = m_transport->send(request);
        record_outbound({
            .adapter = "stripe",
            .operation = std::string{operation},
            .method = request.method,
            .url = request.url,
            .request_body = request.body,
            .response_body = response.body,
            .error = response.error,
            .status = response.status,
            .ok = response.ok(),
        });
        return normalize(response, "stripe");
    }

    [[nodiscard]] static std::string trim_slash(std::string_view url)
    {
        return url.empty() || url.back() != '/'
            ? std::string{url}
            : std::string{url.substr(0, url.size() - 1)};
    }

    Config m_config;
    std::shared_ptr<HttpTransport> m_transport;
};

} // namespace core::adapters
