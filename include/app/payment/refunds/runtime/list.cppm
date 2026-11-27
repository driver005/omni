export module app_payment:refunds_runtime_list;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundListRuntime
{
public:
    explicit RefundListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<::app::model::refunds::RefundListResponse, core::error::AppError>
    list(const RefundsListInput& input) const
    {
        m_context.metrics().record_flow("refund_list");
        m_context.metrics().counter("refunds_list_total").increment();

        const auto result = m_support.refund_list(input.facts, input.request, input.profile_scoped);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return ::app::model::refunds::RefundListResponse{*result};
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
