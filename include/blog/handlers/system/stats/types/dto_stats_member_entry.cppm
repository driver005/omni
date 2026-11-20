module;
export module blog_handlers:stats_dto_stats_member_entry;

import std;
import core_serde;


export namespace blog::dto::stats {

/** @brief Ghost API DTO: StatsMemberEntry. */
class StatsMemberEntry
{
public:
    void set_comped(int value) { comped = std::move(value); }
    [[nodiscard]] const int& get_comped() const noexcept { return comped; }
    void set_date(std::string value) { date = std::move(value); }
    [[nodiscard]] const std::string& get_date() const noexcept { return date; }
    void set_free(int value) { free = std::move(value); }
    [[nodiscard]] const int& get_free() const noexcept { return free; }
    void set_gift(int value) { gift = std::move(value); }
    [[nodiscard]] const int& get_gift() const noexcept { return gift; }
    void set_paid(int value) { paid = std::move(value); }
    [[nodiscard]] const int& get_paid() const noexcept { return paid; }
    void set_paid_canceled(int value) { paid_canceled = std::move(value); }
    [[nodiscard]] const int& get_paid_canceled() const noexcept { return paid_canceled; }
    void set_paid_subscribed(int value) { paid_subscribed = std::move(value); }
    [[nodiscard]] const int& get_paid_subscribed() const noexcept { return paid_subscribed; }

    int comped{0};
    std::string date{};
    int free{0};
    int gift{0};
    int paid{0};
    int paid_canceled{0};
    int paid_subscribed{0};
};

} // namespace blog::dto::stats
