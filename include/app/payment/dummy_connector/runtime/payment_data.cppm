export module app_payment:dummy_connector_runtime_payment_data;
import core_behavior;

import std;
import core_serde;
import interfaces;
import core_error;
import core_cache;
import app_context;
import app_metrics;

import :dummy_connector_input;
import :dummy_connector_result;
import :dummy_connector_runtime_common;

export namespace app::handlers::dummy_connector {

class DummyConnectorPaymentDataRuntime
{
public:
    explicit DummyConnectorPaymentDataRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/dummy_connector.rs:87-105 -> core.rs:44-56 (payment_data).
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    payment_data(const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentDataData{ctx};
        const auto payment = m_support.load_payment(input.payment_id);
        if (!payment) {
            // core.rs:54 -> utils.rs:67 - PaymentNotFound (DC_01).
            return std::unexpected{m_support.values().dc_payment_not_found()};
        }
        m_context.logger().record_flow("DummyPaymentRetrieve");
        m_context.logger().counter("DUMMY_CONNECTOR_PAYMENT_DATA").increment();
        return serde::Value{m_support.payment_response(*payment)};
    }

private:
    ::app::context::AppContext& m_context;
    DummyConnectorRuntimeSupport m_support;
};

} // namespace app::handlers::dummy_connector
