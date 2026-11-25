export module app_handlers_subscription:runtime_pause;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::subscription {

class SubscriptionPauseRuntime
{
public:
    explicit SubscriptionPauseRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // pause_subscription_on_connector (billing_processor_handler.rs:354-383)
    [[nodiscard]] std::expected<PauseSubscriptionResult, core::error::AppError>
    pause(const PauseSubscriptionInput& input) const
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
            "pause_subscription",
            *found,
            input.payload ? input.payload->doc : Json{},
            input.merchant_id
        );
        const std::string status =
            m_support.subscription_status_from_connector(connector, "paused");
        found->status = status;
        found->modified_at = m_support.now_value();
        m_context.repository().update_subscription(m_support.to_store_record(*found));

        std::string paused_at = m_support.now_value();
        if (input.payload) {
            if (const auto value = m_support.json_string(input.payload->root, "pause_at")) {
                paused_at = *value;
            }
        }
        m_support.metrics_for("subscription.pause");
        return PauseSubscriptionResult{m_support.build_pause_response(*found, paused_at)};
    }

private:
    app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
