export module app_handlers_refunds:runtime_update;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::refunds {

class RefundUpdateRuntime
{
public:
    explicit RefundUpdateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsUpdateResult, core::error::AppError>
    update(const RefundsUpdateInput& input) const
    {
        m_context.metrics().record_flow("refund_update");
        m_context.metrics().counter("refunds_update_total").increment();

        const auto result = m_support.refund_update_core(input.request);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return RefundsUpdateResult{m_support.to_refund_response(*result)};
    }

private:
    app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
