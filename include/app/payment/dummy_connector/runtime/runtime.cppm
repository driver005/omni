export module app_payment:dummy_connector_runtime;

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

    [[nodiscard]] std::expected<std::string, core::error::AppError>
    authorize_payment(const AuthorizePaymentInput& input) const
    {
        return m_authorize_payment.authorize_payment(input);
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    complete_payment(const CompletePaymentInput& input) const
    {
        return m_complete_payment.complete_payment(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    payment(const PaymentInput& input) const
    {
        return m_payment.payment(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    payment_data(const PaymentDataInput& input) const
    {
        return m_payment_data.payment_data(input);
    }

    [[nodiscard]] std::
        expected<::app::handlers::dummy_connector::DummyConnectorRefundWire, core::error::AppError>
        refund(const RefundInput& input) const
    {
        return m_refund.refund(input);
    }

    [[nodiscard]] std::
        expected<::app::handlers::dummy_connector::DummyConnectorRefundWire, core::error::AppError>
        refund_data(const RefundDataInput& input) const
    {
        return m_refund_data.refund_data(input);
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
