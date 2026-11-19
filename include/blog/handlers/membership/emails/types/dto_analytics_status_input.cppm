module;
export module blog_handlers:emails_dto_analytics_status_input;

import std;
import core_serde;


export namespace blog::dto::emails {

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



} // namespace blog::dto::emails

template<>
struct serde::Serializable<blog::dto::emails::AnalyticsStatusInput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"jobName", &blog::dto::emails::AnalyticsStatusInput::get_jobName, &blog::dto::emails::AnalyticsStatusInput::set_jobName, serde::FieldOptions{}>{},
            serde::FieldDesc<"running", &blog::dto::emails::AnalyticsStatusInput::get_running, &blog::dto::emails::AnalyticsStatusInput::set_running, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_status_input";
    }
};

