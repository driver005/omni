export module app_auth:verify_connector_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_dto;

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

    [[nodiscard]] core::behavior::Outcome<serde::Value> payment_connector_verify(
        const ::app::dto::verify_connector::VerifyConnectorRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_payment_connector_verify.payment_connector_verify(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentConnectorVerifyRuntime m_payment_connector_verify;
};

} // namespace app::handlers::verify_connector
