export module app_payment:refunds_runtime_retrieve_body;
import core_behavior;

import std;
import interfaces;
import core_error;
import app_context;
import app_context_core;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundRetrieveBodyRuntime
{
public:
    explicit RefundRetrieveBodyRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    retrieve_body(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsRetrieveBodyData{dto, ctx};
        m_context.logger().record_flow(
            input.request.force_sync.value_or(false) ? "refund_retrieve_force_sync"
                                                     : "refund_retrieve"
        );
        m_context.logger().counter("refunds_sync_total").increment();

        const auto outcome =
            m_support.refund_retrieve_core_with_refund_id(input.facts, input.request);
        if (!outcome) {
            return std::unexpected{outcome.error()};
        }
        return m_support.to_refund_response(outcome->record, outcome->raw.has_value());
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
