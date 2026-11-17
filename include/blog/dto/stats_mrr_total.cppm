module;
export module blog_dto:stats_mrr_total;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: StatsMrrTotal. */
class StatsMrrTotal
{
public:
    void set_currency(std::string value) { currency = std::move(value); }
    [[nodiscard]] const std::string& get_currency() const noexcept { return currency; }
    void set_mrr(int value) { mrr = std::move(value); }
    [[nodiscard]] const int& get_mrr() const noexcept { return mrr; }

    std::string currency{"usd"};
    int mrr{1000};
};

} // namespace blog::dto
