export module app_payment:refunds_runtime_filters;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundFiltersRuntime
{
public:
    explicit RefundFiltersRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsFiltersResult, core::error::AppError>
    filters(const RefundsFiltersInput& input) const
    {
        (void)input;
        m_context.metrics().record_flow("refund_filters");
        m_context.metrics().counter("refunds_filters_total").increment();

        return RefundsFiltersResult{
            m_support.get_filters_for_refunds(input.facts, /*profile_scoped=*/false)
        };
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
