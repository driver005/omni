export module app_handlers_revenue_recovery:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_cancel;

export namespace app::handlers::revenue_recovery {

class RevenueRecoveryRuntime
{
public:
    explicit RevenueRecoveryRuntime(app::context::AppContext& context) noexcept :
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
    app::context::AppContext& m_context;
    RevenueRecoveryCancelRuntime m_cancel;
};

} // namespace app::handlers::revenue_recovery
