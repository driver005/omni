export module app_handlers_payments:runtime_payments_capture;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_dto;
import app_store;
import app_connector;
import app_crypto;
import core_cache;
import app_http;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::payments {

class PaymentsCaptureRuntime
{
public:
    explicit PaymentsCaptureRuntime(
        app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<PaymentsCaptureResult, core::error::AppError>
    payments_capture(const PaymentsCaptureInput& input) const
    {
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        const auto pipeline =
            m_support.run_payment_pipeline(input.data, "payments_capture", "capture", "succeeded");
        if (!pipeline.locked) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::DUPLICATE_REQUEST,
                "payment is locked"
            )};
        }
        if (pipeline.decision_rejected) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "routing decision rejected"
            )};
        }
        return PaymentsCaptureResult{m_support.payment_response(pipeline)};
    }

private:
    app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
