module;
export module blog_handlers:stats_dto_stats_mrr_entry;

import std;
import core_serde;


export namespace blog::dto::stats {

/** @brief Ghost API DTO: StatsMrrEntry. */
class StatsMrrEntry
{
public:
    void set_currency(std::string value) { currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return currency; }
    void set_date(std::string value) { date = std::move(value); }
    [[nodiscard]] const std::string& get_date() const noexcept { return date; }
    void set_mrr(int value) { mrr = std::move(value); }
    [[nodiscard]] const int& get_mrr() const noexcept { return mrr; }

    std::string currency{};
    std::string date{};
    int mrr{0};
};

} // namespace blog::dto::stats
