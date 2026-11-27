export module app_payment:refunds_runtime_retrieve;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :refunds_input;
import :refunds_result;
import :refunds_runtime_common;

export namespace app::handlers::refunds {

class RefundRetrieveRuntime
{
public:
    explicit RefundRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<::app::model::refunds::RefundResponse, core::error::AppError>
    retrieve(const RefundsRetrieveInput& input) const
    {
        m_context.metrics().record_flow(
            input.request.get_force_sync().value_or(false) ? "refund_retrieve_force_sync"
                                                           : "refund_retrieve"
        );
        m_context.metrics().counter("refunds_retrieve_total").increment();

        const auto outcome =
            m_support.refund_retrieve_core_with_refund_id(input.facts, input.request);
        if (!outcome) {
            return std::unexpected{outcome.error()};
        }
        return ::app::model::refunds::RefundResponse{m_support.to_refund_response(
            outcome->record,
            input.include_raw_requested || outcome->raw.has_value()
        )};
    }

private:
    ::app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
