module;
#include <rfl.hpp>
export module blog_handlers:dto_analytics_job;

import std;
import core_serde;


export namespace blog::dto {

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



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::AnalyticsJob>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"fetchedThrough", &blog::dto::AnalyticsJob::get_fetchedThrough, &blog::dto::AnalyticsJob::set_fetchedThrough, serde::FieldOptions{}>{},
            serde::FieldDesc<"jobName", &blog::dto::AnalyticsJob::get_jobName, &blog::dto::AnalyticsJob::set_jobName, serde::FieldOptions{}>{},
            serde::FieldDesc<"lagSeconds", &blog::dto::AnalyticsJob::get_lagSeconds, &blog::dto::AnalyticsJob::set_lagSeconds, serde::FieldOptions{}>{},
            serde::FieldDesc<"running", &blog::dto::AnalyticsJob::get_running, &blog::dto::AnalyticsJob::set_running, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_job";
    }
};

