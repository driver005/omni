export module app_payment:payment_methods_runtime_payment_method_session_delete_saved_payment_method;
import core_behavior;

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

class PaymentMethodSessionDeleteSavedPaymentMethodRuntime
{
public:
    explicit PaymentMethodSessionDeleteSavedPaymentMethodRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<
        ::app::model::payment_methods::PaymentMethodDeleteSessionResponse>
    payment_method_session_delete_saved_payment_method(
        const Dto& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        const auto input = PaymentMethodSessionDeleteSavedPaymentMethodData{dto, ctx};
        const auto& record = input.record;
        (void)m_support.call_connector(
            input.session.get_merchant_id(),
            "delete_token",
            record.get_payment_method_id(),
            record.get_vault_id()
        );
        m_context.repository().remove_payment_method(record.get_id());
        m_context.cache().erase("payment_methods.card:" + record.get_payment_method_id());
        m_support.publish_pm_event(
            "payment_method_session.delete_saved",
            record.get_payment_method_id()
        );
        m_context.logger().record_flow("payment_method_session_delete_saved_payment_method");
        m_context.logger()
            .counter("PAYMENT_METHOD_PAYMENT_METHOD_SESSION_DELETE_SAVED_PAYMENT_METHOD")
            .increment();
        ::app::model::payment_methods::PaymentMethodDeleteSessionResponse response{};
        response.set_payment_method_token(input.token);
        return ::app::model::payment_methods::PaymentMethodDeleteSessionResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
