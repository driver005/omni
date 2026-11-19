module;
#include <rfl.hpp>
export module blog_handlers:automations_dto_stats_empty_response;

import std;
import core_serde;


export namespace blog::dto::automations {

/** @brief Ghost API DTO: StatsEmptyResponse. */
class StatsEmptyResponse
{
public:
    void set_stats(std::vector<rfl::Generic> value) { stats = std::move(value); }
    [[nodiscard]] const std::vector<rfl::Generic>& get_stats() const noexcept { return stats; }

    std::vector<rfl::Generic> stats{};
};



} // namespace blog::dto::automations

template<>
struct serde::Serializable<blog::dto::automations::StatsEmptyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"stats", &blog::dto::automations::StatsEmptyResponse::get_stats, &blog::dto::automations::StatsEmptyResponse::set_stats, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "stats_empty_response";
    }
};

