export module app_payment:payments_runtime_confirm_intent_with_external_vault_proxy;

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

import :payments_input;
import :payments_result;
import :payments_runtime_common;

export namespace app::handlers::payments {

class ConfirmIntentWithExternalVaultProxyRuntime
{
public:
    explicit ConfirmIntentWithExternalVaultProxyRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<::app::model::payments::PaymentsResponse, core::error::AppError>
    confirm_intent_with_external_vault_proxy(
        const ConfirmIntentWithExternalVaultProxyInput& input
    ) const
    {
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        const auto pipeline = m_support.run_payment_pipeline(
            input.data,
            "confirm_intent_with_external_vault_proxy",
            "authorize",
            "requires_capture"
        );
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
        return ::app::model::payments::PaymentsResponse{m_support.payment_response(pipeline)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
