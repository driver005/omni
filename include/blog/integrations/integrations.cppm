export module blog_integrations:integrations;

import std;
import core_events;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::integrations {

/** @brief Webhook/notifications fan-out service over core_events. */
class IntegrationService
{
public:
    void subscribe(std::string url, std::string event)
    {
        core::events::WebhookEndpoint endpoint{std::move(url), {}};
        endpoint.subscribe(std::move(event));
        m_dispatcher.add(std::move(endpoint));
    }

    [[nodiscard]] std::vector<std::string> targets(std::string_view event) const
    {
        return m_dispatcher.targets(event);
    }

    void count(std::string_view name)
    {
        m_stats.increment(name);
    }

    [[nodiscard]] std::uint64_t count_of(std::string_view name) const
    {
        return m_stats.value(name);
    }

private:
    core::events::WebhookDispatcher m_dispatcher{};
    core::events::StatsCounter m_stats{};
};

} // namespace blog::integrations

#ifdef CONGELADO_TEST
namespace blog::integrations::tests {
using namespace boost::ut;
suite<"blog_integrations_integrations"> blog_integrations_integrations_suite = []
{
    "fans out webhooks"_test = []
    {
        IntegrationService service;
        service.subscribe("https://x/hook", "post.published");
        expect(service.targets("post.published").size() == 1);
        service.count("requests");
        expect(service.count_of("requests") == 1);
    };
};
} // namespace blog::integrations::tests
#endif
