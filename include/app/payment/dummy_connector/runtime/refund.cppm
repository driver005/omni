export module app_payment:dummy_connector_runtime_refund;

import std;
import interfaces;
import core_error;
import core_cache;
import app_context;
import app_metrics;

import :dummy_connector_input;
import :dummy_connector_result;
import :dummy_connector_runtime_common;

export namespace app::handlers::dummy_connector {

class DummyConnectorRefundRuntime
{
public:
    explicit DummyConnectorRefundRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/dummy_connector.rs:109-128 -> core.rs:156-207 (refund_payment).
    [[nodiscard]] std::
        expected<::app::handlers::dummy_connector::DummyConnectorRefundWire, core::error::AppError>
        refund(const RefundInput& input) const
    {
        // core.rs:173-175 - get_payment_data_from_payment_id.
        auto payment = m_support.load_payment(input.payment_id);
        if (!payment) {
            return std::unexpected{m_support.values().dc_payment_not_found()};
        }

        // types.rs:301-315 - is_eligible_for_refund: amount first, then status.
        const auto eligibility = m_support.is_eligible_for_refund(*payment, input.amount);
        if (!eligibility) {
            return std::unexpected{eligibility.error()};
        }

        // core.rs:179-188 - refund id, decrement, persist payment with payment_ttl.
        const std::string refund_id =
            m_context.ids().payment_id(std::string{REFUND_ID_PREFIX} + "_");
        payment->eligible_amount -= input.amount;
        m_support.save_payment(*payment);

        // core.rs:190-205 - new RefundResponse, store with refund_ttl.
        RefundData refund{};
        refund.status = Status::Succeeded;
        refund.id = refund_id;
        refund.currency = payment->currency;
        refund.created = m_support.now_iso8601();
        refund.payment_amount = payment->amount;
        refund.refund_amount = input.amount;
        m_context.cache().put(
            refund.id,
            core::cache::to_cache_value(m_support.encode_refund_data(refund)),
            m_support.ttl("refund")
        );

        m_context.metrics().record_flow("DummyRefundCreate");
        m_context.metrics().counter("DUMMY_CONNECTOR_REFUND").increment();
        return m_support.refund_response(refund);
    }

private:
    ::app::context::AppContext& m_context;
    DummyConnectorRuntimeSupport m_support;
};

} // namespace app::handlers::dummy_connector
