export module app_webhook:subscription_runtime_get_estimate;

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

class SubscriptionGetEstimateRuntime
{
public:
    explicit SubscriptionGetEstimateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_estimate(const Json& payload, const ::core::behavior::CallContext& ctx) const
    {

        if (const auto failure =
                m_support.find_business_profile(ctx.profile_id, ctx.merchant_id)) {
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

        const std::string plan_id = m_support.json_string(payload, "plan_id").value_or("");
        const std::string item_price_id =
            m_support.json_string(payload, "item_price_id").value_or("");
        SubscriptionRecord seed{};
        seed.subscription_id = m_context.ids().payment_id("sub_");
        seed.merchant_id = ctx.merchant_id;
        seed.profile_id = ctx.profile_id;
        seed.plan_id = plan_id;
        seed.item_price_id = item_price_id;
        Estimate estimate = billing.estimate(
            seed,
            plan_id,
            item_price_id,
            m_support.json_string(payload, "coupon_code").value_or(""),
            ctx.client_secret
        );

        m_support.metrics_for("subscription.get_estimate");
        return m_support.build_estimate_response(estimate, ctx.profile_id);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
