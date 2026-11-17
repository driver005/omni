module;
export module blog_dto:member_subscription_input;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: MemberSubscriptionInput. */
class MemberSubscriptionInput
{
public:
    void set_tier(std::string value) { tier = std::move(value); }
    [[nodiscard]] const std::string& get_tier() const noexcept { return tier; }
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }

    std::string tier{"free"};
    std::string status{"active"};
};

} // namespace blog::dto
