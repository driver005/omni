export module app_payment:refunds_runtime_manual_update;
import core_behavior;

import std;
import core_serde;
import interfaces;
import core_error;
import app_context;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundManualUpdateRuntime
{
public:
    explicit RefundManualUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    manual_update(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsManualUpdateData{dto, ctx};
        m_context.logger().record_flow("refund_manual_update");
        m_context.logger().counter("refunds_manual_update_total").increment();

        if (const auto failure = m_support.refund_manual_update(input.request)) {
            return std::unexpected{*failure};
        }
        return empty_refunds_wire();
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
