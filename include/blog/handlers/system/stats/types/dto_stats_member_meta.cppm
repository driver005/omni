module;
export module blog_handlers:stats_dto_stats_member_meta;

import std;
import core_serde;
import :stats_dto_stats_member_totals;

export namespace blog::dto::stats {

/** @brief Ghost API DTO: StatsMemberMeta. */
class StatsMemberMeta
{
public:
    void set_totals(StatsMemberTotals value)
    {
        totals = std::move(value);
    }

    [[nodiscard]] const StatsMemberTotals& get_totals() const noexcept
    {
        return totals;
    }

    StatsMemberTotals totals{};
};


} // namespace blog::dto::stats

template<>
struct serde::Serializable<blog::dto::stats::StatsMemberMeta>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "totals",
            &blog::dto::stats::StatsMemberMeta::get_totals,
            &blog::dto::stats::StatsMemberMeta::set_totals,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "stats_member_meta";
    }
};
