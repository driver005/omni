export module app_handlers_verify_connector:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_payment_connector_verify;

export namespace app::handlers::verify_connector {

class VerifyConnectorRuntime
{
public:
    explicit VerifyConnectorRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_payment_connector_verify{context}
    {
    }

    [[nodiscard]] std::expected<PaymentConnectorVerifyResult, core::error::AppError>
    payment_connector_verify(const PaymentConnectorVerifyInput& input) const
    {
        return m_payment_connector_verify.payment_connector_verify(input);
    }

private:
    app::context::AppContext& m_context;
    PaymentConnectorVerifyRuntime m_payment_connector_verify;
};

} // namespace app::handlers::verify_connector
