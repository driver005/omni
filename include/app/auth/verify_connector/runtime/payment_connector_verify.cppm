export module app_auth:verify_connector_runtime_payment_connector_verify;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_dto;
import app_connector;
import app_metrics;

import :verify_connector_input;
import :verify_connector_runtime_common;

export namespace app::handlers::verify_connector {

class PaymentConnectorVerifyRuntime
{
    using Error = core::error::AppError;
    using Connector = ::app::dto::verify_connector::Connector;

public:
    explicit PaymentConnectorVerifyRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: payment_connector_verify ─────────────────────────
    // routes/verify_connector.rs:12-34 -> core/verify_connector.rs:22-66.
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    payment_connector_verify(
        const ::app::dto::verify_connector::VerifyConnectorRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        const auto connector =
            ::app::dto::verify_connector::connector_from_wire(request.get_connector_name());
        if (!connector) {
            return std::unexpected{m_support.incorrect_connector_name_given()};
        }
        const auto connector_name =
            std::string{::app::dto::verify_connector::connector_to_wire(*connector)};

        // Upstream core/verify_connector.rs:24-30 — resolve connector from config, mapping a
        // miss to IncorrectConnectorNameGiven.
        auto connector_data = m_support.get_connector_by_name(connector_name);
        if (!connector_data) {
            return std::unexpected{connector_data.error()};
        }

        // Upstream core/verify_connector.rs:32-37 — get_test_card_details + FlowNotSupported.
        auto card_details = m_support.get_test_card_details(*connector);
        if (!card_details) {
            return std::unexpected{card_details.error()};
        }
        if (!card_details->has_value()) {
            return std::unexpected{m_support.flow_not_supported(connector_name)};
        }

        // Upstream core/verify_connector.rs:43-47 / 53-57 — build the structured
        // VerifyConnectorData { connector, connector_auth.foreign_into(), card_details }.
        VerifyConnectorData verify_data{};
        verify_data.connector = connector_data->connector;
        verify_data.connector_auth =
            m_support.foreign_into(request.get_connector_account_details());
        verify_data.card_details = **card_details;

        // The local in-process connector contract stands in for the upstream connector SDK /
        // external PSP integration selected by `boxed_connector.connector`.
        auto selected = m_context.connector_router().select_checked(ctx.merchant_id);
        if (!selected) {
            return std::unexpected{selected.error()};
        }
        const auto connector_impl = *selected;

        // Upstream core/verify_connector.rs:39-66 — dispatch on Connector::Stripe /
        // Connector::Paypal; every other connector already returned FlowNotSupported above.
        core::behavior::Outcome<void> outcome{};
        if (*connector == Connector::stripe) {
            outcome = m_support.stripe_verify(connector_impl, ctx.merchant_id, verify_data);
        } else if (*connector == Connector::paypal) {
            outcome =
                m_support.paypal_get_access_token(connector_impl, ctx.merchant_id, verify_data);
        } else {
            return std::unexpected{m_support.flow_not_supported(connector_name)};
        }
        if (!outcome) {
            return std::unexpected{outcome.error()};
        }

        // Upstream #[instrument(flow = ?Flow::VerifyPaymentConnector)] (routes:12,18) flow marker.
        m_context.metrics().record_flow("VerifyPaymentConnector");
        m_context.metrics().counter("VERIFY_PAYMENT_CONNECTOR").increment();

        // Upstream returns RouterResponse<()> / StatusOk with no payload
        // (core/verify_connector.rs:23,60). The empty serde::Value document is serialized by the
        // handler via Ser::serialize.
        return serde::Value{serde::Value::Object{}};
    }

private:
    ::app::context::AppContext& m_context;
    VerifyConnectorRuntimeSupport m_support;
};

} // namespace app::handlers::verify_connector
