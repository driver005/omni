module;
export module blog_handlers:dto_analytics_status_input;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: AnalyticsStatusInput. */
class AnalyticsStatusInput
{
public:
    void set_jobName(std::string value) { jobName = std::move(value); }
    [[nodiscard]] const std::string& get_jobName() const noexcept { return jobName; }
    void set_running(bool value) { running = std::move(value); }
    [[nodiscard]] const bool& get_running() const noexcept { return running; }

    std::string jobName{};
    bool running{false};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::AnalyticsStatusInput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"jobName", &blog::dto::AnalyticsStatusInput::get_jobName, &blog::dto::AnalyticsStatusInput::set_jobName, serde::FieldOptions{}>{},
            serde::FieldDesc<"running", &blog::dto::AnalyticsStatusInput::get_running, &blog::dto::AnalyticsStatusInput::set_running, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_status_input";
    }
};

