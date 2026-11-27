export module app_payment:refunds_runtime_metadata_update;
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

class RefundMetadataUpdateRuntime
{
public:
    explicit RefundMetadataUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::Refund>
    metadata_update(const ::app::model::Refund& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = RefundsMetadataUpdateData{dto, ctx};
        m_context.logger().record_flow("refund_metadata_update");
        m_context.logger().counter("refunds_metadata_update_total").increment();

        const auto result = m_support.refund_metadata_update_core(input.request);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return m_support.to_refund_response(*result);
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
