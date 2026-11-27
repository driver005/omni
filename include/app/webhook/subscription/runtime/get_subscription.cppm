export module app_webhook:subscription_runtime_get_subscription;

import std;
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

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    get_subscription(const GetSubscriptionInput& input) const
    {
        if (const auto failure =
                m_support.find_business_profile(input.profile_id, input.merchant_id)) {
            return std::unexpected{*failure};
        }
        auto found = m_support.find_subscription(input.subscription_id);
        if (!found) {
            return std::unexpected{m_support.generic_not_found(
                "subscription not found for id: " + input.subscription_id
            )};
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
