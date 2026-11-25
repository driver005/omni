export module app_handlers_payments:runtime_list_payment_methods_for_payments_client;

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

class ListPaymentMethodsForPaymentsClientRuntime
{
public:
    explicit ListPaymentMethodsForPaymentsClientRuntime(
        app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<ListPaymentMethodsForPaymentsClientResult, core::error::AppError>
    list_payment_methods_for_payments_client(
        const ListPaymentMethodsForPaymentsClientInput& input
    ) const
    {
        auto text = m_support.client_payment_methods_response(input.data);
        if (!text) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::UNAUTHORIZED,
                "Invalid client secret"
            )};
        }
        return ListPaymentMethodsForPaymentsClientResult{*text};
    }

private:
    app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
