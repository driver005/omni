export module app_payment:refunds_runtime_reverse;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundReverseRuntime
{
public:
    explicit RefundReverseRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::refunds::RefundResponse>
        reverse(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsReverseData{dto, ctx};
        m_context.metrics().record_flow("refund_reverse");
        m_context.metrics().counter("refunds_reverse_total").increment();

        const auto result = m_support.refund_reverse_core(input.request);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return ::app::model::refunds::RefundResponse{m_support.to_refund_response(*result)};
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
