module;
export module blog_handlers:dto_stats_mrr_response;

import std;
import core_serde;
import :dto_stats_mrr_entry;
import :dto_stats_mrr_meta;

export namespace blog::dto {

/** @brief Ghost API DTO: StatsMrrResponse. */
class StatsMrrResponse
{
public:
    void set_stats(std::vector<StatsMrrEntry> value) { stats = std::move(value); }
    [[nodiscard]] const std::vector<StatsMrrEntry>& get_stats() const noexcept { return stats; }
    void set_meta(StatsMrrMeta value) { meta = std::move(value); }
    [[nodiscard]] const StatsMrrMeta& get_meta() const noexcept { return meta; }

    std::vector<StatsMrrEntry> stats{};
    StatsMrrMeta meta{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::StatsMrrResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"stats", &blog::dto::StatsMrrResponse::get_stats, &blog::dto::StatsMrrResponse::set_stats, serde::FieldOptions{}>{},
            serde::FieldDesc<"meta", &blog::dto::StatsMrrResponse::get_meta, &blog::dto::StatsMrrResponse::set_meta, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "stats_mrr_response";
    }
};

