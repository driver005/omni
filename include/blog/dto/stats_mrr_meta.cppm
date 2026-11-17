module;
export module blog_dto:stats_mrr_meta;

import std;
import core_serde;
import :stats_mrr_total;

export namespace blog::dto {

/** @brief Ghost API DTO: StatsMrrMeta. */
class StatsMrrMeta
{
public:
    void set_totals(std::vector<StatsMrrTotal> value) { totals = std::move(value); }
    [[nodiscard]] const std::vector<StatsMrrTotal>& get_totals() const noexcept { return totals; }

    std::vector<StatsMrrTotal> totals{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::StatsMrrMeta>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"totals", &blog::dto::StatsMrrMeta::get_totals, &blog::dto::StatsMrrMeta::set_totals, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "stats_mrr_meta";
    }
};

