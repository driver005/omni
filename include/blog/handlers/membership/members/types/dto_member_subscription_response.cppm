module;
export module blog_handlers:members_dto_member_subscription_response;

import std;
import core_serde;
import :members_dto_subscription_member;

export namespace blog::dto::members {

/** @brief Ghost API DTO: MemberSubscriptionResponse. */
class MemberSubscriptionResponse
{
public:
    void set_members(std::vector<SubscriptionMember> value) { members = std::move(value); }
    [[nodiscard]] const std::vector<SubscriptionMember>& get_members() const noexcept { return members; }

    std::vector<SubscriptionMember> members{};
};



} // namespace blog::dto::members

template<>
struct serde::Serializable<blog::dto::members::MemberSubscriptionResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"members", &blog::dto::members::MemberSubscriptionResponse::get_members, &blog::dto::members::MemberSubscriptionResponse::set_members, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "member_subscription_response";
    }
};

