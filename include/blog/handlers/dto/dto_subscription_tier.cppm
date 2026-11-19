module;
export module blog_handlers:dto_subscription_tier;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: SubscriptionTier. */
class SubscriptionTier
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }

    std::string name{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::SubscriptionTier>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &blog::dto::SubscriptionTier::get_name, &blog::dto::SubscriptionTier::set_name, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "subscription_tier";
    }
};

