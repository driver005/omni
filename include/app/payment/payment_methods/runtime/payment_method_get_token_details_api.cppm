export module app_payment:payment_methods_runtime_payment_method_get_token_details_api;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :payment_methods_input;
import :payment_methods_result;
import :payment_methods_runtime_common;

export namespace app::handlers::payment_methods {

class PaymentMethodGetTokenDetailsApiRuntime
{
public:
    explicit PaymentMethodGetTokenDetailsApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::PaymentMethodGetTokenDetailsResponse>
    payment_method_get_token_details_api(const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentMethodGetTokenDetailsApiData{ctx};
        m_context.metrics().record_flow("payment_method_get_token_details_api");
        m_context.metrics()
            .counter("PAYMENT_METHOD_PAYMENT_METHOD_GET_TOKEN_DETAILS_API")
            .increment();
        ::app::model::payment_methods::PaymentMethodGetTokenDetailsResponse response{};
        ::app::model::payment_methods::GlobalPaymentMethodIdValue id{};
        id.set_value(input.payment_method_id);
        response.set_id(std::move(id));
        response.set_payment_method_token(input.temporary_token);
        return ::app::model::payment_methods::PaymentMethodGetTokenDetailsResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
