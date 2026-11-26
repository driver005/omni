export module app_webhook:subscription_runtime_confirm;

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

class SubscriptionConfirmRuntime
{
public:
    explicit SubscriptionConfirmRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ConfirmSubscriptionResult, core::error::AppError>
    confirm(const ConfirmSubscriptionInput& input) const
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
        auto found = m_support.find_subscription(input.subscription_id);
        if (!found) {
            return std::unexpected{m_support.generic_not_found(
                "subscription not found for id: " + input.subscription_id
            )};
        }

        InvoiceView invoice = m_support.latest_invoice(found->subscription_id);
        if (invoice.invoice_id.empty()) {
            return std::unexpected{m_support.subscription_error("Get Latest Invoice")};
        }
        if (invoice.payment_intent_id.empty()) {
            return std::unexpected{m_support.missing_field("payment_intent_id")};
        }

        // confirm_payment (invoice_handler.rs:203-229) — local payments substitute
        PaymentView payment = m_support.confirm_payment(
            found->subscription_id,
            invoice.payment_intent_id,
            *found,
            m_support.estimate_for(*found)
        );

        BillingProcessor billing{m_context};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            return std::unexpected{*failure};
        }
        if (const auto failure = m_support.find_customer(found->customer_id, found->merchant_id)) {
            return std::unexpected{*failure};
        }
        m_support.create_connector_customer(
            *found,
            input.payload.doc
        ); // create_customer_on_connector

        ConnectorResponse connector = m_support.call_billing(
            "create_subscription_on_connector",
            *found,
            input.payload.doc,
            found->merchant_id
        );
        const std::string connector_subscription_id =
            m_support.connector_reference(connector, "bpsub_" + found->subscription_id);
        const std::string connector_invoice_id =
            m_support.connector_reference(connector, "pinv_" + found->subscription_id);

        // update_invoice(payment + status) (core.rs:401-413)
        invoice.payment_intent_id = payment.payment_id;
        invoice.payment_method_id = payment.payment_method_id;
        invoice.status = "invoice_paid";
        invoice.connector_invoice_id = connector_invoice_id;
        m_support.store_invoice(invoice);

        // create_invoice_sync_job (invoice_handler.rs:286-307)
        m_support
            .create_invoice_sync_job(*found, invoice, connector_invoice_id, billing.connector_name);

        // update_subscription(connector id, payment method, status) (core.rs:424-437)
        found->status = m_support.subscription_status_from_connector(connector, "active");
        found->connector_subscription_id = connector_subscription_id;
        found->payment_method_id = payment.payment_method_id;
        found->modified_at = m_support.now_value();
        m_context.repository().update_subscription(m_support.to_store_record(*found));

        m_support.metrics_for("subscription.confirm");
        return ConfirmSubscriptionResult{
            m_support.build_confirm_response(*found, invoice, payment)
        };
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
