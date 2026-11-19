module;
#include <rfl.hpp>
export module blog_handlers:emails_dto_analytics_job;

import std;
import core_serde;


export namespace blog::dto::emails {

/** @brief Ghost API DTO: AnalyticsJob. */
class AnalyticsJob
{
public:
    void set_fetchedThrough(rfl::Generic value) { fetchedThrough = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_fetchedThrough() const noexcept { return fetchedThrough; }
    void set_jobName(std::string value) { jobName = std::move(value); }
    [[nodiscard]] const std::string& get_jobName() const noexcept { return jobName; }
    void set_lagSeconds(rfl::Generic value) { lagSeconds = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_lagSeconds() const noexcept { return lagSeconds; }
    void set_running(bool value) { running = std::move(value); }
    [[nodiscard]] const bool& get_running() const noexcept { return running; }

    rfl::Generic fetchedThrough{};
    std::string jobName{};
    rfl::Generic lagSeconds{};
    bool running{false};
};



} // namespace blog::dto::emails

template<>
struct serde::Serializable<blog::dto::emails::AnalyticsJob>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"fetchedThrough", &blog::dto::emails::AnalyticsJob::get_fetchedThrough, &blog::dto::emails::AnalyticsJob::set_fetchedThrough, serde::FieldOptions{}>{},
            serde::FieldDesc<"jobName", &blog::dto::emails::AnalyticsJob::get_jobName, &blog::dto::emails::AnalyticsJob::set_jobName, serde::FieldOptions{}>{},
            serde::FieldDesc<"lagSeconds", &blog::dto::emails::AnalyticsJob::get_lagSeconds, &blog::dto::emails::AnalyticsJob::set_lagSeconds, serde::FieldOptions{}>{},
            serde::FieldDesc<"running", &blog::dto::emails::AnalyticsJob::get_running, &blog::dto::emails::AnalyticsJob::set_running, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_job";
    }
};

