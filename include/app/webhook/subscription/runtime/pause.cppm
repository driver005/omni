export module app_webhook:subscription_runtime_pause;

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

class SubscriptionPauseRuntime
{
public:
    explicit SubscriptionPauseRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // pause_subscription_on_connector (billing_processor_handler.rs:354-383)
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    pause(
        const std::optional<ParsedPayload>& payload,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const std::string subscription_id =
            ctx.path_segments.size() > 1 ? ctx.path_segments.at(1) : std::string{};

        if (const auto failure =
                m_support.find_business_profile(ctx.profile_id, ctx.merchant_id)) {
            return std::unexpected{*failure};
        }
        auto found = m_support.find_subscription(subscription_id);
        if (!found) {
            return std::unexpected{m_support.generic_not_found(
                "subscription not found for id: " + subscription_id
            )};
        }
        BillingProcessor billing{m_context};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            return std::unexpected{*failure};
        }

        auto connector = m_support.call_billing(
            "pause_subscription",
            *found,
            payload ? payload->doc : Json{},
            ctx.merchant_id
        );
        const std::string status =
            m_support.subscription_status_from_connector(connector, "paused");
        found->status = status;
        found->modified_at = m_support.now_value();
        m_context.repository().update_subscription(m_support.to_store_record(*found));

        std::string paused_at = m_support.now_value();
        if (payload) {
            if (const auto value = m_support.json_string(payload->root, "pause_at")) {
                paused_at = *value;
            }
        }
        m_support.metrics_for("subscription.pause");
        return m_support.build_pause_response(*found, paused_at);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
