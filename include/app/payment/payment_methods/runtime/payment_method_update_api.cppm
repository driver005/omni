export module app_payment:payment_methods_runtime_payment_method_update_api;

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

class PaymentMethodUpdateApiRuntime
{
public:
    explicit PaymentMethodUpdateApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::PaymentMethodUpdateResponse>
        payment_method_update_api(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentMethodUpdateApiData{dto, ctx};
        auto record = input.record;
        if (input.request.get_card()) {
            const auto vault_response = m_support.call_connector(
                record.get_merchant_id(),
                "tokenize",
                input.payment_method_id,
                serde::Ser::encode_json(*input.request.get_card())
            );
            if (!vault_response.get_reference_id().empty()) {
                record.set_vault_id(vault_response.get_reference_id());
            }
            m_context.cache().put(
                "payment_methods.card:" + input.payment_method_id,
                core::cache::to_cache_value(
                    m_support.encrypt_data(serde::Ser::encode_json(*input.request.get_card()))
                )
            );
        }
        record.set_status("active");
        if (!m_context.repository().update_payment_method(record)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INTERNAL_ERROR, "persistence failed")
            };
        }
        m_support.publish_pm_event("payment_method.update", input.payment_method_id);
        m_context.metrics().record_flow("payment_method_update_api");
        m_context.metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_UPDATE_API").increment();
        ::app::model::payment_methods::PaymentMethodUpdateResponse response{};
        response.set_payment_method_id(input.payment_method_id);
        ::app::model::payment_methods::PaymentMethodStatusValue status{};
        status.set_value(record.get_status());
        response.set_status(std::move(status));
        response.set_update_status(::app::model::payment_methods::UpdateStatus::Success);
        response.set_updated_payment_method_data(true);
        return ::app::model::payment_methods::PaymentMethodUpdateResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
