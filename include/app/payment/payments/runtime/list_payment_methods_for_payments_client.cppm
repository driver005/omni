export module app_payment:payments_runtime_list_payment_methods_for_payments_client;

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

class ListPaymentMethodsForPaymentsClientRuntime
{
public:
    explicit ListPaymentMethodsForPaymentsClientRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentsClientMethodsResponse>
        list_payment_methods_for_payments_client(const ::app::model::payments::PaymentsRequest& dto, const core::behavior::CallContext& ctx) const
    {
        const PaymentsRequestData input = m_support.request_data(dto, ctx);
        auto text = m_support.client_payment_methods_response(input);
        if (!text) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::UNAUTHORIZED,
                "Invalid client secret"
            )};
        }
        return *text;
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
