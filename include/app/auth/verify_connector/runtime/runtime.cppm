export module app_auth:verify_connector_runtime;

import std;
import core_error;
import core_serde;
import app_context;

import :verify_connector_input;
import :verify_connector_runtime_common;
import :verify_connector_runtime_payment_connector_verify;

export namespace app::handlers::verify_connector {

class VerifyConnectorRuntime
{
public:
    explicit VerifyConnectorRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_payment_connector_verify{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    payment_connector_verify(const PaymentConnectorVerifyInput& input) const
    {
        return m_payment_connector_verify.payment_connector_verify(input);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentConnectorVerifyRuntime m_payment_connector_verify;
};

} // namespace app::handlers::verify_connector
