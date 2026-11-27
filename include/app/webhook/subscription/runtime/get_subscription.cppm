export module app_webhook:subscription_runtime_get_subscription;

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

class SubscriptionGetSubscriptionRuntime
{
public:
    explicit SubscriptionGetSubscriptionRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_subscription(const ::core::behavior::CallContext& ctx) const
    {
        const std::string subscription_id =
            ctx.path_segments.size() > 1 ? ctx.path_segments.at(1) : std::string{};

        if (const auto failure = m_support.find_business_profile(ctx.profile_id, ctx.merchant_id)) {
            return std::unexpected{*failure};
        }
        auto found = m_support.find_subscription(subscription_id);
        if (!found) {
            return std::unexpected{
                m_support.generic_not_found("subscription not found for id: " + subscription_id)
            };
        }
        m_support.metrics_for("subscription.get");
        // to_subscription_response(subscription, None, None) — no payment/invoice on this route.
        return m_support
            .build_subscription_response(*found, std::nullopt, std::nullopt, std::nullopt);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
