export module app_payment:dummy_connector_runtime;
import core_behavior;

import std;
import core_serde;
import core_error;
import app_context;

import :dummy_connector_input;
import :dummy_connector_result;
import :dummy_connector_runtime_common;
import :dummy_connector_runtime_authorize_payment;
import :dummy_connector_runtime_complete_payment;
import :dummy_connector_runtime_payment;
import :dummy_connector_runtime_payment_data;
import :dummy_connector_runtime_refund;
import :dummy_connector_runtime_refund_data;

export namespace app::handlers::dummy_connector {

class DummyConnectorRuntime
{
public:
    explicit DummyConnectorRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_authorize_payment{context},
        m_complete_payment{context},
        m_payment{context},
        m_payment_data{context},
        m_refund{context},
        m_refund_data{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::string>
    authorize_payment(const core::behavior::CallContext& ctx) const
    {
        return m_authorize_payment.authorize_payment(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    complete_payment(const core::behavior::CallContext& ctx) const
    {
        return m_complete_payment.complete_payment(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    payment(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_payment.payment(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    payment_data(const core::behavior::CallContext& ctx) const
    {
        return m_payment_data.payment_data(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        ::app::handlers::dummy_connector::DummyConnectorRefundWire>
    refund(const core::behavior::CallContext& ctx) const
    {
        return m_refund.refund(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        ::app::handlers::dummy_connector::DummyConnectorRefundWire>
    refund_data(const core::behavior::CallContext& ctx) const
    {
        return m_refund_data.refund_data(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    DummyConnectorAuthorizePaymentRuntime m_authorize_payment;
    DummyConnectorCompletePaymentRuntime m_complete_payment;
    DummyConnectorPaymentRuntime m_payment;
    DummyConnectorPaymentDataRuntime m_payment_data;
    DummyConnectorRefundRuntime m_refund;
    DummyConnectorRefundDataRuntime m_refund_data;
};

} // namespace app::handlers::dummy_connector
