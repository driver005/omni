export module app_handlers_refunds:runtime_list_platform;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::refunds {

class RefundListPlatformRuntime
{
public:
    explicit RefundListPlatformRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsListPlatformResult, core::error::AppError>
    list_platform(const RefundsListPlatformInput& input) const
    {
        m_context.metrics().record_flow("refund_platform_list");
        m_context.metrics().counter("refunds_platform_list_total").increment();

        const auto result = m_support.refund_list_for_platform(input.facts, input.request);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return RefundsListPlatformResult{*result};
    }

private:
    app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
