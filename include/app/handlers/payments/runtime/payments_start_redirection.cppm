export module app_handlers_payments:runtime_payments_start_redirection;

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

class PaymentsStartRedirectionRuntime
{
public:
    explicit PaymentsStartRedirectionRuntime(
        app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<PaymentsStartRedirectionResult, core::error::AppError>
    payments_start_redirection(const PaymentsStartRedirectionInput& input) const
    {
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(m_support.local_redirect_url(input.data, service_status));
        return PaymentsStartRedirectionResult{response};
    }

private:
    app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
