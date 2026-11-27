export module app_payment:refunds_runtime_filters_profile;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundFiltersProfileRuntime
{
public:
    explicit RefundFiltersProfileRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundListFilters>
    filters_profile(const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsFiltersProfileData{ctx};
        m_context.metrics().record_flow("refund_profile_filters");
        m_context.metrics().counter("refunds_profile_filters_total").increment();

        return ::app::model::refunds::RefundListFilters{
            m_support.get_filters_for_refunds(input.facts, /*profile_scoped=*/true)
        };
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
