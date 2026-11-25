export module app_handlers_refunds:runtime_filter_platform;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::refunds {

class RefundFilterPlatformRuntime
{
public:
    explicit RefundFilterPlatformRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsFilterPlatformResult, core::error::AppError>
    filter_platform(const RefundsFilterPlatformInput& input) const
    {
        m_context.metrics().record_flow("refund_platform_filter");
        m_context.metrics().counter("refunds_platform_filter_total").increment();

        const auto result = m_support.get_platform_refund_filters(input.facts);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return RefundsFilterPlatformResult{*result};
    }

private:
    app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
