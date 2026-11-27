export module app_webhook:subscription_runtime_create_and_confirm;

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

class SubscriptionCreateAndConfirmRuntime
{
public:
    explicit SubscriptionCreateAndConfirmRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // core::create_and_confirm_subscription (core.rs:176-307)
    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    create_and_confirm(const CreateAndConfirmSubscriptionInput& input) const
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
        record.status = "created";
        record.plan_id = m_support.json_string(input.payload.root, "plan_id").value_or("");
        record.item_price_id = input.item_price_id;
        record.coupon = m_support.json_string(input.payload.root, "coupon_code").value_or("");
        record.merchant_reference_id =
            m_support.json_string(input.payload.root, "merchant_reference_id").value_or("");
        record.created_at = m_support.now_value();
        record.modified_at = record.created_at;
        record.client_secret = m_support.generate_client_secret(subscription_id);
        record.merchant_connector_id = billing.merchant_connector_id;
        auto stored = m_context.repository().create_subscription(m_support.to_store_record(record));
        if (!stored) {
            return std::unexpected{m_support.internal_error()};
        }
        record.id = stored->get_id();

        // create_customer_on_connector + update_connector_customer_id_in_customer (core.rs:219-240)
        m_support.create_connector_customer(record, input.payload.doc);

        // create_subscription_on_connector (core.rs:242-249)
        ConnectorResponse connector = m_support.call_billing(
            "create_subscription_on_connector",
            record,
            input.payload.doc,
            merchant
        );
        record.connector_subscription_id =
            m_support.connector_reference(connector, "bpsub_" + subscription_id);

        // get_amount_and_currency((None,None), invoice_details) (invoice_handler.rs:103-114)
        Estimate estimate =
            m_support.estimate_from_connector(connector, record.plan_id, record.item_price_id);

        // create_and_confirm_payment (invoice_handler.rs:159-201)
        PaymentView payment = m_support.create_payment(record, estimate, true, input.payload, true);

        // create_invoice_entry + create_invoice_sync_job (core.rs:259-283)
        InvoiceView invoice = m_support.create_invoice(
            record,
            billing.merchant_connector_id,
            estimate,
            payment.payment_id,
            "invoice_created",
            m_support.connector_reference(connector, "pinv_" + subscription_id)
        );
        m_support.create_invoice_sync_job(
            record,
            invoice,
            invoice.connector_invoice_id,
            billing.connector_name
        );

        // update_subscription(connector id, payment method, status) (core.rs:285-298)
        record.status = m_support.subscription_status_from_connector(connector, "active");
        record.payment_method_id = payment.payment_method_id;
        m_context.repository().update_subscription(m_support.to_store_record(record));

        m_support.metrics_for("subscription.create_and_confirm");
        return m_support.build_confirm_response(record, invoice, payment);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
