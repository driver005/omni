module;
export module blog_handlers:emails_dto_analytics_status_input;

import std;
import core_serde;


export namespace blog::dto::emails {

/** @brief Ghost API DTO: AnalyticsStatusInput (data:['id'], options:['begin','end']). */
class AnalyticsStatusInput
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_begin(std::string value) { begin = std::move(value); }
    [[nodiscard]] const std::string& get_begin() const noexcept { return begin; }
    void set_end(std::string value) { end = std::move(value); }
    [[nodiscard]] const std::string& get_end() const noexcept { return end; }
    void set_jobName(std::string value) { jobName = std::move(value); }
    [[nodiscard]] const std::string& get_jobName() const noexcept { return jobName; }
    void set_running(bool value) { running = std::move(value); }
    [[nodiscard]] const bool& get_running() const noexcept { return running; }

    std::string id{};
    std::string begin{};
    std::string end{};
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
            serde::FieldDesc<"id", &blog::dto::emails::AnalyticsStatusInput::get_id, &blog::dto::emails::AnalyticsStatusInput::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"begin", &blog::dto::emails::AnalyticsStatusInput::get_begin, &blog::dto::emails::AnalyticsStatusInput::set_begin, serde::FieldOptions{}>{},
            serde::FieldDesc<"end", &blog::dto::emails::AnalyticsStatusInput::get_end, &blog::dto::emails::AnalyticsStatusInput::set_end, serde::FieldOptions{}>{},
            serde::FieldDesc<"jobName", &blog::dto::emails::AnalyticsStatusInput::get_jobName, &blog::dto::emails::AnalyticsStatusInput::set_jobName, serde::FieldOptions{}>{},
            serde::FieldDesc<"running", &blog::dto::emails::AnalyticsStatusInput::get_running, &blog::dto::emails::AnalyticsStatusInput::set_running, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_status_input";
    }
};
