export module app_payment:refunds_runtime_filter_list;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundFilterListRuntime
{
public:
    explicit RefundFilterListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsFilterListResult, core::error::AppError>
    filter_list(const RefundsFilterListInput& input) const
    {
        m_context.metrics().record_flow("refund_filter_list");
        m_context.metrics().counter("refunds_filter_list_total").increment();

        // core/refunds.rs:2132-2148 — filter_refund_by_meta_constraints(merchant, time_range).
        const auto records = m_support.merchant_records(input.facts, /*profile_scoped=*/false);
        return RefundsFilterListResult{m_support.filter_list_metadata(
            m_support.apply_time_range(records, input.start_time, input.end_time)
        )};
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
