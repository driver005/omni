export module app_webhook:subscription_runtime_get_estimate;

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

class SubscriptionGetEstimateRuntime
{
public:
    explicit SubscriptionGetEstimateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    get_estimate(const GetEstimateInput& input) const
    {
        if (const auto failure =
                m_support.find_business_profile(input.profile_id, input.merchant_id)) {
            return std::unexpected{*failure};
        }
        if (!input.client_secret.empty()) {
            if (const auto failure =
                    m_support.validate_client_secret(input.client_secret, input.merchant_id)) {
                return std::unexpected{*failure};
            }
        }
        BillingProcessor billing{m_context};
        if (const auto failure = billing.create(input.profile_id, input.merchant_id)) {
            return std::unexpected{*failure};
        }

        const std::string plan_id = m_support.json_string(input.payload, "plan_id").value_or("");
        const std::string item_price_id =
            m_support.json_string(input.payload, "item_price_id").value_or("");
        SubscriptionRecord seed{};
        seed.subscription_id = m_context.ids().payment_id("sub_");
        seed.merchant_id = input.merchant_id;
        seed.profile_id = input.profile_id;
        seed.plan_id = plan_id;
        seed.item_price_id = item_price_id;
        Estimate estimate = billing.estimate(
            seed,
            plan_id,
            item_price_id,
            m_support.json_string(input.payload, "coupon_code").value_or(""),
            input.client_secret
        );

        m_support.metrics_for("subscription.get_estimate");
        return m_support.build_estimate_response(estimate, input.profile_id);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
