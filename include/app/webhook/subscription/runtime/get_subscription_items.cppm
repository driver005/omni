export module app_webhook:subscription_runtime_get_subscription_items;

import std;
import core_behavior;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_store;
import app_crypto;
import app_connector;
import app_metrics;
import app_events;

import :subscription_input;
import :subscription_runtime_common;

export namespace app::handlers::subscription {

[[nodiscard]] inline std::uint32_t query_unsigned(
    const std::unordered_map<std::string, std::string>& query,
    std::string_view key,
    std::uint32_t fallback
)
{
    const auto it = query.find(std::string{key});
    if (it == query.end()) {
        return fallback;
    }
    try {
        return static_cast<std::uint32_t>(std::stoul(it->second));
    } catch (...) {
        return fallback;
    }
}

class SubscriptionGetSubscriptionItemsRuntime
{
public:
    explicit SubscriptionGetSubscriptionItemsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // get_subscription_items + get_subscription_item_prices (core.rs:151-172)
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_subscription_items(const ::core::behavior::CallContext& ctx) const
    {
        const auto limit = static_cast<std::uint32_t>(query_unsigned(ctx.query, "limit", 10));
        const auto offset = static_cast<std::uint32_t>(query_unsigned(ctx.query, "offset", 0));
        const std::string item_type =
            ctx.query.contains("item_type") ? ctx.query.at("item_type") : std::string{"plan"};

        if (const auto failure = m_support.find_business_profile(ctx.profile_id, ctx.merchant_id)) {
            return std::unexpected{*failure};
        }
        if (!ctx.client_secret.empty()) {
            if (const auto failure =
                    m_support.validate_client_secret(ctx.client_secret, ctx.merchant_id)) {
                return std::unexpected{*failure};
            }
        }
        BillingProcessor billing{m_context};
        if (const auto failure = billing.create(ctx.profile_id, ctx.merchant_id)) {
            return std::unexpected{*failure};
        }

        auto items = billing.get_items(ctx.profile_id, limit, offset, item_type);
        m_support.metrics_for("subscription.get_subscription_items");
        return m_support.build_items_response(items);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
