export module app_webhook:subscription_runtime_resume;

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

class SubscriptionResumeRuntime
{
public:
    explicit SubscriptionResumeRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    resume(const ResumeSubscriptionInput& input) const
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
            "resume_subscription",
            *found,
            input.payload ? input.payload->doc : Json{},
            input.merchant_id
        );
        const std::string status =
            m_support.subscription_status_from_connector(connector, "active");
        found->status = status;
        found->modified_at = m_support.now_value();
        m_context.repository().update_subscription(m_support.to_store_record(*found));

        std::string next_billing_at = m_support.now_value();
        if (input.payload) {
            if (const auto value = m_support.json_string(input.payload->root, "resume_date")) {
                next_billing_at = *value;
            }
        }
        m_support.metrics_for("subscription.resume");
        return m_support.build_resume_response(*found, next_billing_at);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
