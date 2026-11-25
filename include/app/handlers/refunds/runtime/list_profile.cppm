export module app_handlers_refunds:runtime_list_profile;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::refunds {

class RefundListProfileRuntime
{
public:
    explicit RefundListProfileRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsListProfileResult, core::error::AppError>
    list_profile(const RefundsListProfileInput& input) const
    {
        m_context.metrics().record_flow("refund_profile_list");
        m_context.metrics().counter("refunds_profile_list_total").increment();

        const auto result =
            m_support.refund_list(input.facts, input.request, /*profile_scoped=*/true);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return RefundsListProfileResult{*result};
    }

private:
    app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
