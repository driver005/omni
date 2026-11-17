module;
#include <rfl.hpp>
export module blog_dto:stats_empty_response;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: StatsEmptyResponse. */
class StatsEmptyResponse
{
public:
    void set_stats(std::vector<rfl::Generic> value) { stats = std::move(value); }
    [[nodiscard]] const std::vector<rfl::Generic>& get_stats() const noexcept { return stats; }

    std::vector<rfl::Generic> stats{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::StatsEmptyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"stats", &blog::dto::StatsEmptyResponse::get_stats, &blog::dto::StatsEmptyResponse::set_stats, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "stats_empty_response";
    }
};

