module;
export module blog_dto:member_subscription_response;

import std;
import core_serde;
import :subscription_member;

export namespace blog::dto {

/** @brief Ghost API DTO: MemberSubscriptionResponse. */
class MemberSubscriptionResponse
{
public:
    void set_members(std::vector<SubscriptionMember> value) { members = std::move(value); }
    [[nodiscard]] const std::vector<SubscriptionMember>& get_members() const noexcept { return members; }

    std::vector<SubscriptionMember> members{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::MemberSubscriptionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"members", &blog::dto::MemberSubscriptionResponse::get_members, &blog::dto::MemberSubscriptionResponse::set_members, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "member_subscription_response";
    }
};

