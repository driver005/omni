export module app_payment:payments_runtime_payments_finish_redirection;

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

class PaymentsFinishRedirectionRuntime
{
public:
    explicit PaymentsFinishRedirectionRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsRedirectionResponse>
        payments_finish_redirection(const ::app::model::payments::PaymentsRequest& dto, const core::behavior::CallContext& ctx) const
    {
        const PaymentsRequestData input = m_support.request_data(dto, ctx);
        if (const auto err = m_support.ensure_payment_exists(input)) {
            return std::unexpected{*err};
        }
        std::string service_status{"pending"};
        ::app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(m_support.local_redirect_url(input, service_status));
        return ::app::model::payments::PaymentsRedirectionResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
