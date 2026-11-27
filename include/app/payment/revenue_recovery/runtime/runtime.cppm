export module app_payment:revenue_recovery_runtime;
import core_behavior;

import std;
import core_error;
import app_context;

import :revenue_recovery_input;
import :revenue_recovery_result;
import :revenue_recovery_runtime_cancel;

export namespace app::handlers::revenue_recovery {

class RevenueRecoveryRuntime
{
public:
    explicit RevenueRecoveryRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_cancel{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::RecoveryPaymentsResponse>
    cancel(const core::behavior::CallContext& ctx) const
    {
        return m_cancel.cancel(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryCancelRuntime m_cancel;
};

} // namespace app::handlers::revenue_recovery
