export module app_payment:refunds_runtime_aggregates_profile;
import core_behavior;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundAggregatesProfileRuntime
{
public:
    explicit RefundAggregatesProfileRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    aggregates_profile(const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsAggregatesProfileData{ctx};
        m_context.logger().record_flow("refund_profile_aggregates");
        m_context.logger().counter("refunds_profile_aggregates_total").increment();

        return m_support.get_aggregates_for_refunds(
            input.facts,
            input.start_time,
            input.end_time,
            /*profile_scoped=*/true
        );
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
