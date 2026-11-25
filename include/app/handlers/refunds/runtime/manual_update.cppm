export module app_handlers_refunds:runtime_manual_update;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::refunds {

class RefundManualUpdateRuntime
{
public:
    explicit RefundManualUpdateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsManualUpdateResult, core::error::AppError>
    manual_update(const RefundsManualUpdateInput& input) const
    {
        m_context.metrics().record_flow("refund_manual_update");
        m_context.metrics().counter("refunds_manual_update_total").increment();

        if (const auto failure = m_support.refund_manual_update(input.request)) {
            return std::unexpected{*failure};
        }
        return RefundsManualUpdateResult{RefundsManualUpdateResult::empty()};
    }

private:
    app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
