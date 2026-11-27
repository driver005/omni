export module app_payment:refunds_runtime_list_platform;
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

class RefundListPlatformRuntime
{
public:
    explicit RefundListPlatformRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_platform(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsListPlatformData{dto, ctx};
        m_context.logger().record_flow("refund_platform_list");
        m_context.logger().counter("refunds_platform_list_total").increment();

        const auto result = m_support.refund_list_for_platform(input.facts, input.request);
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
