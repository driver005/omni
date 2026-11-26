export module app_payment:revenue_recovery_runtime;

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

    [[nodiscard]] std::expected<RevenueRecoveryCancelResult, core::error::AppError>
    cancel(const RevenueRecoveryCancelInput& input) const
    {
        return m_cancel.cancel(input);
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryCancelRuntime m_cancel;
};

} // namespace app::handlers::revenue_recovery
