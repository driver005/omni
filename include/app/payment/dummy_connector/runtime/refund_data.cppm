export module app_payment:dummy_connector_runtime_refund_data;

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

class DummyConnectorRefundDataRuntime
{
public:
    explicit DummyConnectorRefundDataRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/dummy_connector.rs:132-150 -> core.rs:210-234 (refund_data).
    [[nodiscard]] std::
        expected<::app::handlers::dummy_connector::DummyConnectorRefundWire, core::error::AppError>
        refund_data(const RefundDataInput& input) const
    {
        const auto stored = m_context.cache().get(input.refund_id);
        if (!stored.hit) {
            // core.rs:226-232 - RefundNotFound (DC_05).
            return std::unexpected{m_support.values().dc_refund_not_found()};
        }
        const auto refund = m_support.decode_refund_data(stored.value_or<std::string>(""));
        if (!refund) {
            return std::unexpected{m_support.values().dc_internal()};
        }
        m_context.metrics().record_flow("DummyRefundRetrieve");
        m_context.metrics().counter("DUMMY_CONNECTOR_REFUND_DATA").increment();
        return m_support.refund_data_response(*refund);
    }

private:
    ::app::context::AppContext& m_context;
    DummyConnectorRuntimeSupport m_support;
};

} // namespace app::handlers::dummy_connector
