export module app_webhook:subscription_runtime_update_subscription;

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

class SubscriptionUpdateSubscriptionRuntime
{
public:
    explicit SubscriptionUpdateSubscriptionRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> update_subscription(
        const ParsedPayload& payload,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const std::string subscription_id =
            ctx.path_segments.size() > 1 ? ctx.path_segments.at(1) : std::string{};
        const std::string plan_id = m_support.json_string(payload.root, "plan_id").value_or("");
        const std::string item_price_id =
            m_support.json_string(payload.root, "item_price_id").value_or("");

        if (const auto failure = m_support.find_business_profile(ctx.profile_id, ctx.merchant_id)) {
            return std::unexpected{*failure};
        }
        auto found = m_support.find_subscription(subscription_id);
        if (!found) {
            return std::unexpected{
                m_support.generic_not_found("subscription not found for id: " + subscription_id)
            };
        }
        InvoiceView invoice = m_support.latest_invoice(found->subscription_id);
        if (invoice.invoice_id.empty()) {
            return std::unexpected{m_support.subscription_error("Get Latest Invoice")};
        }

        // update_subscription(plan_id, item_price_id) (core.rs:669-677)
        found->plan_id = plan_id;
        found->item_price_id = item_price_id;
        found->modified_at = m_support.now_value();
        m_context.repository().update_subscription(m_support.to_store_record(*found));

        BillingProcessor billing{m_context};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            return std::unexpected{*failure};
        }
        Estimate estimate =
            billing.estimate(*found, plan_id, item_price_id, "", found->client_secret);

        // update_invoice + update_payment (core.rs:698-718)
        invoice.amount = estimate.amount;
        invoice.currency = estimate.currency;
        m_support.store_invoice(invoice);
        m_support.update_payment_amount(invoice.payment_intent_id, estimate);

        m_support.metrics_for("subscription.update");
        // core ends with get_subscription(...) -> to_subscription_response(None, None)
        return m_support
            .build_subscription_response(*found, std::nullopt, std::nullopt, std::nullopt);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
