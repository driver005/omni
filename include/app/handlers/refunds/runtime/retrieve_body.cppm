export module app_handlers_refunds:runtime_retrieve_body;

import std;
import interfaces;
import core_error;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::refunds {

class RefundRetrieveBodyRuntime
{
public:
    explicit RefundRetrieveBodyRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RefundsRetrieveBodyResult, core::error::AppError>
    retrieve_body(const RefundsRetrieveBodyInput& input) const
    {
        m_context.metrics().record_flow(
            input.request.get_force_sync().value_or(false) ? "refund_retrieve_force_sync"
                                                           : "refund_retrieve"
        );
        m_context.metrics().counter("refunds_sync_total").increment();

        const auto outcome =
            m_support.refund_retrieve_core_with_refund_id(input.facts, input.request);
        if (!outcome) {
            return std::unexpected{outcome.error()};
        }
        return RefundsRetrieveBodyResult{
            m_support.to_refund_response(outcome->record, outcome->raw.has_value())
        };
    }

private:
    app::context::AppContext& m_context;
    RefundRuntimeSupport m_support;
};

} // namespace app::handlers::refunds
