export module app_handlers_fraud_check:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_frm_fulfillment;

export namespace app::handlers::fraud_check {

class FraudCheckRuntime
{
public:
    explicit FraudCheckRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_frm_fulfillment{context}
    {
    }

    [[nodiscard]] std::expected<FrmFulfillmentResult, core::error::AppError>
    frm_fulfillment(const FrmFulfillmentInput& input) const
    {
        return m_frm_fulfillment.frm_fulfillment(input);
    }

private:
    app::context::AppContext& m_context;
    FrmFulfillmentRuntime m_frm_fulfillment;
};

} // namespace app::handlers::fraud_check
