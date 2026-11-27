export module app_payment:payment_methods_runtime_payment_method_session_update_saved_payment_method;

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

class PaymentMethodSessionUpdateSavedPaymentMethodRuntime
{
public:
    explicit PaymentMethodSessionUpdateSavedPaymentMethodRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::PaymentMethodResponse>
        payment_method_session_update_saved_payment_method(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentMethodSessionUpdateSavedPaymentMethodData{dto, ctx};
        auto record = input.record;
        const auto vault = m_support.call_connector(
            input.session.get_merchant_id(),
            "tokenize",
            record.get_payment_method_id(),
            input.request.get_payment_method_token().value_or("")
        );
        if (!vault.get_reference_id().empty()) {
            record.set_vault_id(vault.get_reference_id());
        }
        m_context.repository().update_payment_method(record);
        m_support.publish_pm_event(
            "payment_method_session.update_saved",
            record.get_payment_method_id()
        );
        m_context.metrics().record_flow("payment_method_session_update_saved_payment_method");
        m_context.metrics()
            .counter("PAYMENT_METHOD_PAYMENT_METHOD_SESSION_UPDATE_SAVED_PAYMENT_METHOD")
            .increment();
        return ::app::model::payment_methods::PaymentMethodResponse{
            m_support.response_from_record(record)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
