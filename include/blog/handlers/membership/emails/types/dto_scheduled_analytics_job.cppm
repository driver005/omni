module;
export module blog_handlers:emails_dto_scheduled_analytics_job;

import std;
import core_serde;

export namespace blog::dto::emails {

/** @brief Ghost API DTO: ScheduledAnalyticsJob. */
class ScheduledAnalyticsJob
{
public:
    void set_jobName(std::string value)
    {
        jobName = std::move(value);
    }

    [[nodiscard]] const std::string& get_jobName() const noexcept
    {
        return jobName;
    }

    void set_running(bool value)
    {
        running = std::move(value);
    }

    [[nodiscard]] const bool& get_running() const noexcept
    {
        return running;
    }

    std::string jobName{};
    bool running{false};
};


} // namespace blog::dto::emails

template<>
struct serde::Serializable<blog::dto::emails::ScheduledAnalyticsJob>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "jobName",
                &blog::dto::emails::ScheduledAnalyticsJob::get_jobName,
                &blog::dto::emails::ScheduledAnalyticsJob::set_jobName,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "running",
                &blog::dto::emails::ScheduledAnalyticsJob::get_running,
                &blog::dto::emails::ScheduledAnalyticsJob::set_running,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "scheduled_analytics_job";
    }
};
