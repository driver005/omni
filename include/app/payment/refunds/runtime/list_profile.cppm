export module app_payment:refunds_runtime_list_profile;
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

class RefundListProfileRuntime
{
public:
    explicit RefundListProfileRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_profile(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsListProfileData{dto, ctx};
        m_context.logger().record_flow("refund_profile_list");
        m_context.logger().counter("refunds_profile_list_total").increment();

        const auto result =
            m_support.refund_list(input.facts, input.request, /*profile_scoped=*/true);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return *result;
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
