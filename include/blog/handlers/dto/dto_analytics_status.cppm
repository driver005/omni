module;
export module blog_handlers:dto_analytics_status;

import std;
import core_serde;
import :dto_analytics_job;
import :dto_scheduled_analytics_job;

export namespace blog::dto {

/** @brief Ghost API DTO: AnalyticsStatus. */
class AnalyticsStatus
{
public:
    void set_latest(AnalyticsJob value) { latest = std::move(value); }
    [[nodiscard]] const AnalyticsJob& get_latest() const noexcept { return latest; }
    void set_latestOpened(AnalyticsJob value) { latestOpened = std::move(value); }
    [[nodiscard]] const AnalyticsJob& get_latestOpened() const noexcept { return latestOpened; }
    void set_missing(AnalyticsJob value) { missing = std::move(value); }
    [[nodiscard]] const AnalyticsJob& get_missing() const noexcept { return missing; }
    void set_scheduled(ScheduledAnalyticsJob value) { scheduled = std::move(value); }
    [[nodiscard]] const ScheduledAnalyticsJob& get_scheduled() const noexcept { return scheduled; }

    AnalyticsJob latest{};
    AnalyticsJob latestOpened{};
    AnalyticsJob missing{};
    ScheduledAnalyticsJob scheduled{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::AnalyticsStatus>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"latest", &blog::dto::AnalyticsStatus::get_latest, &blog::dto::AnalyticsStatus::set_latest, serde::FieldOptions{}>{},
            serde::FieldDesc<"latestOpened", &blog::dto::AnalyticsStatus::get_latestOpened, &blog::dto::AnalyticsStatus::set_latestOpened, serde::FieldOptions{}>{},
            serde::FieldDesc<"missing", &blog::dto::AnalyticsStatus::get_missing, &blog::dto::AnalyticsStatus::set_missing, serde::FieldOptions{}>{},
            serde::FieldDesc<"scheduled", &blog::dto::AnalyticsStatus::get_scheduled, &blog::dto::AnalyticsStatus::set_scheduled, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_status";
    }
};

