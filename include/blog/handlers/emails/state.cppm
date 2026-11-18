export module blog_handlers:emails_state;

import std;
import blog_dto;
import blog_model;
import blog_handlers_middleware;
import :mutation;

export namespace blog::api {

/** @brief Mutable email analytics / batch state owned by the emails domain. */
class EmailsState
{
public:
    EmailsState()
    {
        reset_analytics();
    }

    /** @brief GET /emails/:id/analytics. */
    std::pair<int, std::string> emails_analytics_get(std::string_view /*id*/) const
    {
        return {200, blog::handlers::middleware::encode(m_analytics)};
    }

    /** @brief PUT /emails/:id/analytics — flips a job's `running` flag. */
    std::pair<int, std::string> emails_analytics_put(std::string_view /*id*/, std::string_view body)
    {
        const auto input = blog::handlers::middleware::decode<blog::dto::AnalyticsStatusInput>(body);
        if (input) {
            auto* job = job_for(input->jobName);
            if (job != nullptr) {
                job->running = input->running;
            }
        }
        return {200, blog::handlers::middleware::encode(m_analytics)};
    }

    /** @brief DELETE /emails/analytics — resets all job flags. */
    std::pair<int, std::string> emails_analytics_delete()
    {
        reset_analytics();
        return {200, "{}"};
    }

    /** @brief GET /emails/:id/batches. */
    std::pair<int, std::string> emails_batches(std::string_view /*id*/) const
    {
        return {200, std::format(
            R"({{"batches":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            blog::dto::GhostFixtures::batch()
        )};
    }

    /** @brief GET /emails/:id/recipient-failures. */
    std::pair<int, std::string> emails_failures(std::string_view /*id*/) const
    {
        return {200, std::format(
            R"({{"failures":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            blog::dto::GhostFixtures::failure()
        )};
    }

private:
    void reset_analytics()
    {
        m_analytics = blog::dto::AnalyticsStatus{};
        m_analytics.latest.jobName = "email-analytics-latest-others";
        m_analytics.latestOpened.jobName = "email-analytics-latest-opened";
        m_analytics.missing.jobName = "email-analytics-missing";
        m_analytics.scheduled.jobName = "email-analytics-scheduled";
    }

    [[nodiscard]] blog::dto::AnalyticsJob* job_for(std::string_view name)
    {
        if (name == "email-analytics-latest-opened") return &m_analytics.latestOpened;
        if (name == "email-analytics-missing") return &m_analytics.missing;
        return &m_analytics.latest;
    }

    blog::dto::AnalyticsStatus m_analytics{};
};

} // namespace blog::api
