export module core_events:webhook_dispatcher;

import std;
import :webhook;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::events {

/** @brief Fans an event out to all subscribed webhook endpoints. */
class WebhookDispatcher
{
public:
    void add(WebhookEndpoint endpoint)
    {
        m_endpoints.push_back(std::move(endpoint));
    }

    [[nodiscard]] std::vector<std::string> targets(std::string_view event) const
    {
        std::vector<std::string> result;
        for (const WebhookEndpoint& endpoint: m_endpoints) {
            if (endpoint.accepts(event)) {
                result.push_back(endpoint.url());
            }
        }
        return result;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_endpoints.size();
    }

    void clear() noexcept
    {
        m_endpoints.clear();
    }

private:
    std::vector<WebhookEndpoint> m_endpoints{};
};

} // namespace core::events

#ifdef CONGELADO_TEST
namespace core::events::tests {
using namespace boost::ut;

suite<"core_events_webhook_dispatcher"> core_events_webhook_dispatcher_suite = []
{
    "fans events out to subscribers"_test = []
    {
        WebhookDispatcher dispatcher;
        WebhookEndpoint endpoint{"https://a/hook", "s"};
        endpoint.subscribe("post.published");
        dispatcher.add(std::move(endpoint));
        expect(dispatcher.size() == 1);
        const auto targets = dispatcher.targets("post.published");
        expect(targets.size() == 1);
        expect(targets[0] == "https://a/hook");
        expect(dispatcher.targets("other").empty());
    };
};

} // namespace core::events::tests
#endif
