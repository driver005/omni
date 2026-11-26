export module app_payment:payment_methods_runtime_payment_method_delete_api;

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

class PaymentMethodDeleteApiRuntime
{
public:
    explicit PaymentMethodDeleteApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<PaymentMethodDeleteApiResult, core::error::AppError>
    payment_method_delete_api(const PaymentMethodDeleteApiInput& input) const
    {
        const auto& record = input.record;
        const auto response_vault = m_support.call_connector(
            record.get_merchant_id(),
            "delete_token",
            input.payment_method_id,
            record.get_vault_id()
        );
        (void)response_vault;
        if (!m_context.repository().remove_payment_method(record.get_id())) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INTERNAL_ERROR, "persistence failed")
            };
        }
        m_context.cache().erase("payment_methods.client_secret:" + input.payment_method_id);
        m_context.cache().erase("payment_methods.card:" + input.payment_method_id);
        m_support.publish_pm_event("payment_method.delete", input.payment_method_id);
        m_context.metrics().record_flow("payment_method_delete_api");
        m_context.metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_DELETE_API").increment();
        ::app::model::payment_methods::PaymentMethodDeleteResponse response{};
        response.set_payment_method_id(input.payment_method_id);
        response.set_deleted(true);
        return PaymentMethodDeleteApiResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
