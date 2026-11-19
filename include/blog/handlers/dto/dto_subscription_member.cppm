module;
#include <rfl.hpp>
export module blog_handlers:dto_subscription_member;

import std;
import core_serde;
import :dto_member_subscription;

export namespace blog::dto {

/** @brief Ghost API DTO: SubscriptionMember. */
class SubscriptionMember
{
public:
    void set_comped(bool value) { comped = std::move(value); }
    [[nodiscard]] const bool& get_comped() const noexcept { return comped; }
    void set_current_subscription(rfl::Generic value) { current_subscription = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_current_subscription() const noexcept { return current_subscription; }
    void set_email(std::string value) { email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return email; }
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_name(rfl::Generic value) { name = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_name() const noexcept { return name; }
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    void set_subscriptions(std::vector<MemberSubscription> value) { subscriptions = std::move(value); }
    [[nodiscard]] const std::vector<MemberSubscription>& get_subscriptions() const noexcept { return subscriptions; }

    bool comped{false};
    rfl::Generic current_subscription{};
    std::string email{};
    std::string id{};
    rfl::Generic name{};
    std::string status{"paid"};
    std::vector<MemberSubscription> subscriptions{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::SubscriptionMember>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"comped", &blog::dto::SubscriptionMember::get_comped, &blog::dto::SubscriptionMember::set_comped, serde::FieldOptions{}>{},
            serde::FieldDesc<"current_subscription", &blog::dto::SubscriptionMember::get_current_subscription, &blog::dto::SubscriptionMember::set_current_subscription, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &blog::dto::SubscriptionMember::get_email, &blog::dto::SubscriptionMember::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"id", &blog::dto::SubscriptionMember::get_id, &blog::dto::SubscriptionMember::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::dto::SubscriptionMember::get_name, &blog::dto::SubscriptionMember::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::dto::SubscriptionMember::get_status, &blog::dto::SubscriptionMember::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"subscriptions", &blog::dto::SubscriptionMember::get_subscriptions, &blog::dto::SubscriptionMember::set_subscriptions, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "subscription_member";
    }
};

