module;
export module blog_handlers:emails_dto_analytics_status;

import std;
import core_serde;
import :emails_dto_analytics_job;
import :emails_dto_scheduled_analytics_job;

export namespace blog::dto::emails {

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



} // namespace blog::dto::emails

template<>
struct serde::Serializable<blog::dto::emails::AnalyticsStatus>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"latest", &blog::dto::emails::AnalyticsStatus::get_latest, &blog::dto::emails::AnalyticsStatus::set_latest, serde::FieldOptions{}>{},
            serde::FieldDesc<"latestOpened", &blog::dto::emails::AnalyticsStatus::get_latestOpened, &blog::dto::emails::AnalyticsStatus::set_latestOpened, serde::FieldOptions{}>{},
            serde::FieldDesc<"missing", &blog::dto::emails::AnalyticsStatus::get_missing, &blog::dto::emails::AnalyticsStatus::set_missing, serde::FieldOptions{}>{},
            serde::FieldDesc<"scheduled", &blog::dto::emails::AnalyticsStatus::get_scheduled, &blog::dto::emails::AnalyticsStatus::set_scheduled, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "analytics_status";
    }
};

