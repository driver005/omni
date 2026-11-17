module;
export module blog_dto:stats_member_totals;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: StatsMemberTotals. */
class StatsMemberTotals
{
public:
    void set_comped(int value) { comped = std::move(value); }
    [[nodiscard]] const int& get_comped() const noexcept { return comped; }
    void set_free(int value) { free = std::move(value); }
    [[nodiscard]] const int& get_free() const noexcept { return free; }
    void set_gift(int value) { gift = std::move(value); }
    [[nodiscard]] const int& get_gift() const noexcept { return gift; }
    void set_paid(int value) { paid = std::move(value); }
    [[nodiscard]] const int& get_paid() const noexcept { return paid; }

    int comped{0};
    int free{3};
    int gift{0};
    int paid{5};
};

} // namespace blog::dto
