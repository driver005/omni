export module blog_handlers:emails_operations;

import std;
import core_cache;
import blog_context;
import blog_dto;
import blog_handlers_middleware;

export namespace blog::api::emails {



class EmailsOperations
{
public:
    explicit EmailsOperations(core::cache::Cache& cache) noexcept :
        m_cache{cache}
    {
    }

[[nodiscard]] std::pair<int, std::string> emails_analytics_get()
{
    return {200, blog::handlers::middleware::encode(load_analytics(m_cache))};
}

[[nodiscard]] std::pair<int, std::string> emails_analytics_put(
    std::string_view body
)
{
    auto analytics = load_analytics(m_cache);
    const auto input = blog::handlers::middleware::decode<blog::dto::AnalyticsStatusInput>(body);
    if (input) {
        if (auto* job = job_for(analytics, input->jobName)) {
            job->running = input->running;
        }
    }
    store_analytics(m_cache, analytics);
    return {200, blog::handlers::middleware::encode(analytics)};
}

[[nodiscard]] std::pair<int, std::string> emails_analytics_delete()
{
    m_cache.erase(kAnalyticsKey);
    return {200, "{}"};
}

[[nodiscard]] std::pair<int, std::string> emails_batches()
{
    return {200, std::format(
        R"({{"batches":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
        blog::dto::GhostFixtures::batch()
    )};
}

[[nodiscard]] std::pair<int, std::string> emails_failures()
{
    return {200, std::format(
        R"({{"failures":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
        blog::dto::GhostFixtures::failure()
    )};
}

private:
static constexpr std::string_view kAnalyticsKey{"emails:analytics"};

[[nodiscard]] static blog::dto::AnalyticsStatus load_analytics(core::cache::Cache& cache)
{
    const auto stored = core::cache::value_as<std::string>(cache.get(kAnalyticsKey).value);
    if (auto analytics = blog::handlers::middleware::decode<blog::dto::AnalyticsStatus>(stored)) {
        return *analytics;
    }
    blog::dto::AnalyticsStatus analytics;
    analytics.latest.jobName = "email-analytics-latest-others";
    analytics.latestOpened.jobName = "email-analytics-latest-opened";
    analytics.missing.jobName = "email-analytics-missing";
    analytics.scheduled.jobName = "email-analytics-scheduled";
    return analytics;
}

static void store_analytics(core::cache::Cache& cache, const blog::dto::AnalyticsStatus& analytics)
{
    cache.put(kAnalyticsKey,
              core::cache::to_cache_value(blog::handlers::middleware::encode(analytics)),
              core::cache::Ttl{}.never(),
              {"emails"});
}

[[nodiscard]] static blog::dto::AnalyticsJob* job_for(
    blog::dto::AnalyticsStatus& analytics,
    std::string_view name
)
{
    if (name == "email-analytics-latest-opened") return &analytics.latestOpened;
    if (name == "email-analytics-missing") return &analytics.missing;
    return &analytics.latest;
}

    core::cache::Cache& m_cache;
};

} // namespace blog::api::emails
