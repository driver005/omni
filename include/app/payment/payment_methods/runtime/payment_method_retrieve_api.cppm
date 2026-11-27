export module app_payment:payment_methods_runtime_payment_method_retrieve_api;
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

class PaymentMethodRetrieveApiRuntime
{
public:
    explicit PaymentMethodRetrieveApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::PaymentMethodResponse>
    payment_method_retrieve_api(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentMethodRetrieveApiData{dto, ctx};
        auto record = input.record;
        if (input.force_sync) {
            const auto sync = m_support.call_connector(
                record.get_merchant_id(),
                "sync",
                input.payment_method_id,
                record.get_vault_id()
            );
            if (!sync.get_status().empty()) {
                record.set_status(sync.get_status());
                m_context.repository().update_payment_method(record);
            }
        }
        m_context.logger().record_flow("payment_method_retrieve_api");
        m_context.logger().counter("PAYMENT_METHOD_PAYMENT_METHOD_RETRIEVE_API").increment();
        return ::app::model::payment_methods::PaymentMethodResponse{
            m_support.response_from_record(record)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
