export module app_webhook:subscription_runtime_cancel;

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

class SubscriptionCancelRuntime
{
public:
    explicit SubscriptionCancelRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    cancel(const CancelSubscriptionInput& input) const
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
        BillingProcessor billing{m_context};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            return std::unexpected{*failure};
        }

        auto connector = m_support.call_billing(
            "cancel_subscription",
            *found,
            input.payload ? input.payload->doc : Json{},
            input.merchant_id
        );
        const std::string status =
            m_support.subscription_status_from_connector(connector, "cancelled");
        found->status = status;
        found->modified_at = m_support.now_value();
        m_context.repository().update_subscription(m_support.to_store_record(*found));

        std::string cancelled_at = m_support.now_value();
        if (input.payload) {
            if (const auto value = m_support.json_string(input.payload->root, "cancel_at")) {
                cancelled_at = *value;
            }
        }
        m_support.metrics_for("subscription.cancel");
        return m_support.build_cancel_response(*found, cancelled_at);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
