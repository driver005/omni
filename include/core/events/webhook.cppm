export module core_events:webhook;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::events {

/** @brief One webhook endpoint subscribed to a set of event names. */
class WebhookEndpoint
{
public:
    WebhookEndpoint() = default;

    WebhookEndpoint(std::string url, std::string secret) :
        m_url{std::move(url)},
        m_secret{std::move(secret)}
    {
    }

    [[nodiscard]] const std::string& url() const noexcept
    {
        return m_url;
    }

    [[nodiscard]] const std::string& secret() const noexcept
    {
        return m_secret;
    }

    WebhookEndpoint& subscribe(std::string event)
    {
        m_events.insert(std::move(event));
        return *this;
    }

    [[nodiscard]] bool accepts(std::string_view event) const noexcept
    {
        return m_events.contains(std::string{event});
    }

    [[nodiscard]] std::size_t subscription_count() const noexcept
    {
        return m_events.size();
    }

private:
    std::string m_url{};
    std::string m_secret{};
    std::set<std::string> m_events{};
};

} // namespace core::events

#ifdef CONGELADO_TEST
namespace core::events::tests {
using namespace boost::ut;

suite<"core_events_webhook"> core_events_webhook_suite = []
{
    "subscribes to event names"_test = []
    {
        WebhookEndpoint endpoint{"https://example.com/hook", "secret"};
        endpoint.subscribe("post.published");
        expect(endpoint.url() == "https://example.com/hook");
        expect(endpoint.accepts("post.published"));
        expect(!endpoint.accepts("member.created"));
        expect(endpoint.subscription_count() == 1);
    };
};

} // namespace core::events::tests
#endif
