export module app_webhook:subscription_runtime_create;

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
import :subscription_result;
import :subscription_runtime_common;

export namespace app::handlers::subscription {

class SubscriptionCreateRuntime
{
public:
    explicit SubscriptionCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // core::create_subscription (core.rs:31-123)
    [[nodiscard]] std::expected<CreateSubscriptionResult, core::error::AppError>
    create(const CreateSubscriptionInput& input) const
    {
        const auto merchant = input.merchant_id;
        if (const auto failure = m_support.find_business_profile(input.profile_id, merchant)) {
            return std::unexpected{*failure};
        }
        if (const auto failure = m_support.find_customer(input.customer_id, merchant)) {
            return std::unexpected{*failure};
        }
        BillingProcessor billing{m_context};
        if (const auto failure = billing.create(input.profile_id, merchant)) {
            return std::unexpected{*failure};
        }

        const std::string subscription_id = m_context.ids().payment_id("sub_");
        SubscriptionRecord record{};
        record.subscription_id = subscription_id;
        record.merchant_id = merchant;
        record.profile_id = input.profile_id;
        record.customer_id = input.customer_id;
        record.status =
            "created"; // SubscriptionStatus::Created.to_string() -> "Created" (subscription.rs:54)
        record.plan_id = m_support.json_string(input.payload.root, "plan_id").value_or("");
        record.item_price_id = input.item_price_id;
        record.coupon = m_support.json_string(input.payload.root, "coupon_code").value_or("");
        record.merchant_reference_id =
            m_support.json_string(input.payload.root, "merchant_reference_id").value_or("");
        record.created_at = m_support.now_value();
        record.modified_at = record.created_at;
        record.client_secret =
            m_support.generate_client_secret(subscription_id); // generate_and_set_client_secret
        record.merchant_connector_id = billing.merchant_connector_id;

        auto stored = m_context.repository().create_subscription(m_support.to_store_record(record));
        if (!stored) {
            return std::unexpected{m_support.internal_error()};
        }
        record.id = stored->get_id();

        // get_subscription_estimate -> amount/currency (core.rs:68-88)
        Estimate estimate = billing.estimate(
            record,
            m_support.json_string(input.payload.root, "plan_id").value_or(""),
            input.item_price_id,
            "",
            record.client_secret
        );

        // create_payment_with_confirm_false (invoice_handler.rs:116-143)
        PaymentView payment =
            m_support.create_payment(record, estimate, false, input.payload, false);

        // create_invoice_entry (invoice_handler.rs:41-79), status = InvoiceCreated ->
        // "invoice_created"
        InvoiceView invoice = m_support.create_invoice(
            record,
            billing.merchant_connector_id,
            estimate,
            payment.payment_id,
            "invoice_created",
            ""
        );

        // update_subscription(payment_method_id, plan_id, item_price_id) (core.rs:105-114)
        record.payment_method_id = payment.payment_method_id;
        m_context.repository().update_subscription(m_support.to_store_record(record));

        m_support.metrics_for("subscription.create");
        return CreateSubscriptionResult{
            m_support.build_subscription_response(record, std::nullopt, payment, invoice)
        };
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
