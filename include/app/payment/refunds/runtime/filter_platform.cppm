export module app_payment:refunds_runtime_filter_platform;
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

class RefundFilterPlatformRuntime
{
public:
    explicit RefundFilterPlatformRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    filter_platform(const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsFilterPlatformData{ctx};
        m_context.logger().record_flow("refund_platform_filter");
        m_context.logger().counter("refunds_platform_filter_total").increment();

        const auto result = m_support.get_platform_refund_filters(input.facts);
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
