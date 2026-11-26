export module app_payment:payments_runtime_payments_redirect_response_with_creds_identifier;

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

class PaymentsRedirectResponseWithCredsIdentifierRuntime
{
public:
    explicit PaymentsRedirectResponseWithCredsIdentifierRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::
        expected<PaymentsRedirectResponseWithCredsIdentifierResult, core::error::AppError>
        payments_redirect_response_with_creds_identifier(
            const PaymentsRedirectResponseWithCredsIdentifierInput& input
        ) const
    {
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        std::string service_status{"pending"};
        ::app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(m_support.local_redirect_url(input.data, service_status));
        return PaymentsRedirectResponseWithCredsIdentifierResult{response};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
