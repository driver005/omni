export module app_handlers_refunds:runtime_aggregates;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::refunds {

class RefundAggregatesRuntime
{
public:
    explicit RefundAggregatesRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsAggregatesResult, core::error::AppError>
    aggregates(const RefundsAggregatesInput& input) const
    {
        m_context.metrics().record_flow("refund_aggregates");
        m_context.metrics().counter("refunds_aggregates_total").increment();

        return RefundsAggregatesResult{m_support.get_aggregates_for_refunds(
            input.facts,
            input.start_time,
            input.end_time,
            /*profile_scoped=*/false
        )};
    }

private:
    app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
