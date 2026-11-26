export module app_payment:refunds_runtime_metadata_update;

import std;
import interfaces;
import core_error;
import app_context;
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

    [[nodiscard]] std::expected<RefundsMetadataUpdateResult, core::error::AppError>
    metadata_update(const RefundsMetadataUpdateInput& input) const
    {
        m_context.metrics().record_flow("refund_metadata_update");
        m_context.metrics().counter("refunds_metadata_update_total").increment();

        const auto result = m_support.refund_metadata_update_core(input.refund_id, input.request);
        if (!result) {
            return std::unexpected{result.error()};
        }
        return RefundsMetadataUpdateResult{m_support.to_refund_response(*result)};
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
