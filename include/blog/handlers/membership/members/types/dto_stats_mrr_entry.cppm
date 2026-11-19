module;
export module blog_handlers:members_dto_stats_mrr_entry;

import std;
import core_serde;


export namespace blog::dto::members {

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

    std::string currency{"usd"};
    std::string date{"1970-01-01"};
    int mrr{0};
};

} // namespace blog::dto::members
