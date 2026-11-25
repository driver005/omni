module;
#include <rfl.hpp>
export module blog_handlers:members_dto_member_subscription;

import std;
import core_serde;
import :members_dto_subscription_tier;

export namespace blog::dto::members {

/** @brief Ghost API DTO: MemberSubscription. */
class MemberSubscription
{
public:
    void set_cancel_at_period_end(bool value)
    {
        cancel_at_period_end = std::move(value);
    }

    [[nodiscard]] const bool& get_cancel_at_period_end() const noexcept
    {
        return cancel_at_period_end;
    }

    void set_cancellation_reason(rfl::Generic value)
    {
        cancellation_reason = std::move(value);
    }

    [[nodiscard]] const rfl::Generic& get_cancellation_reason() const noexcept
    {
        return cancellation_reason;
    }

    void set_current_period_end(std::string value)
    {
        current_period_end = std::move(value);
    }

    [[nodiscard]] const std::string& get_current_period_end() const noexcept
    {
        return current_period_end;
    }

    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_status(std::string value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    void set_tier(SubscriptionTier value)
    {
        tier = std::move(value);
    }

    [[nodiscard]] const SubscriptionTier& get_tier() const noexcept
    {
        return tier;
    }

    bool cancel_at_period_end{false};
    rfl::Generic cancellation_reason{};
    std::string current_period_end{};
    std::string id{};
    std::string status{};
    SubscriptionTier tier{};
};


} // namespace blog::dto::members

template<>
struct serde::Serializable<blog::dto::members::MemberSubscription>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "cancel_at_period_end",
                &blog::dto::members::MemberSubscription::get_cancel_at_period_end,
                &blog::dto::members::MemberSubscription::set_cancel_at_period_end,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "cancellation_reason",
                &blog::dto::members::MemberSubscription::get_cancellation_reason,
                &blog::dto::members::MemberSubscription::set_cancellation_reason,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "current_period_end",
                &blog::dto::members::MemberSubscription::get_current_period_end,
                &blog::dto::members::MemberSubscription::set_current_period_end,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "id",
                &blog::dto::members::MemberSubscription::get_id,
                &blog::dto::members::MemberSubscription::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &blog::dto::members::MemberSubscription::get_status,
                &blog::dto::members::MemberSubscription::set_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tier",
                &blog::dto::members::MemberSubscription::get_tier,
                &blog::dto::members::MemberSubscription::set_tier,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "member_subscription";
    }
};
