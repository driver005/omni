module;
export module blog_handlers:stats_dto_stats_member_response;

import std;
import core_serde;
import :stats_dto_stats_member_entry;
import :stats_dto_stats_member_meta;

export namespace blog::dto::stats {

/** @brief Ghost API DTO: StatsMemberResponse. */
class StatsMemberResponse
{
public:
    void set_stats(std::vector<StatsMemberEntry> value) { stats = std::move(value); }
    [[nodiscard]] const std::vector<StatsMemberEntry>& get_stats() const noexcept { return stats; }
    void set_meta(StatsMemberMeta value) { meta = std::move(value); }
    [[nodiscard]] const StatsMemberMeta& get_meta() const noexcept { return meta; }

    std::vector<StatsMemberEntry> stats{};
    StatsMemberMeta meta{};
};



} // namespace blog::dto::stats

template<>
struct serde::Serializable<blog::dto::stats::StatsMemberResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"stats", &blog::dto::stats::StatsMemberResponse::get_stats, &blog::dto::stats::StatsMemberResponse::set_stats, serde::FieldOptions{}>{},
            serde::FieldDesc<"meta", &blog::dto::stats::StatsMemberResponse::get_meta, &blog::dto::stats::StatsMemberResponse::set_meta, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "stats_member_response";
    }
};

