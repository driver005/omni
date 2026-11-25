export module app_handlers_refunds:runtime_aggregates_profile;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::refunds {

class RefundAggregatesProfileRuntime
{
public:
    explicit RefundAggregatesProfileRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsAggregatesProfileResult, core::error::AppError>
    aggregates_profile(const RefundsAggregatesProfileInput& input) const
    {
        m_context.metrics().record_flow("refund_profile_aggregates");
        m_context.metrics().counter("refunds_profile_aggregates_total").increment();

        return RefundsAggregatesProfileResult{m_support.get_aggregates_for_refunds(
            input.facts,
            input.start_time,
            input.end_time,
            /*profile_scoped=*/true
        )};
    }

private:
    app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
