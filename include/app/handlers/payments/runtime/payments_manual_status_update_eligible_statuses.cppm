export module app_handlers_payments:runtime_payments_manual_status_update_eligible_statuses;

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

class PaymentsManualStatusUpdateEligibleStatusesRuntime
{
public:
    explicit PaymentsManualStatusUpdateEligibleStatusesRuntime(
        app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::
        expected<PaymentsManualStatusUpdateEligibleStatusesResult, core::error::AppError>
        payments_manual_status_update_eligible_statuses(
            const PaymentsManualStatusUpdateEligibleStatusesInput& input
        ) const
    {
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        return PaymentsManualStatusUpdateEligibleStatusesResult{
            m_support.payment_response_view(input.data)
        };
    }

private:
    app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
