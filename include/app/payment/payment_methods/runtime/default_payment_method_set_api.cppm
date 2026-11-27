export module app_payment:payment_methods_runtime_default_payment_method_set_api;

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

class DefaultPaymentMethodSetApiRuntime
{
public:
    explicit DefaultPaymentMethodSetApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<
        ::app::model::payment_methods::CustomerDefaultPaymentMethodResponse,
        core::error::AppError>
    default_payment_method_set_api(const DefaultPaymentMethodSetApiInput& input) const
    {
        m_context.cache().put(
            "payment_methods.default:" + input.customer_id,
            core::cache::to_cache_value(input.payment_method_id)
        );
        m_support.publish_pm_event("payment_method.set_default", input.payment_method_id);
        m_context.metrics().record_flow("default_payment_method_set_api");
        m_context.metrics().counter("PAYMENT_METHOD_DEFAULT_PAYMENT_METHOD_SET_API").increment();
        ::app::model::payment_methods::CustomerDefaultPaymentMethodResponse response{};
        response.set_default_payment_method_id(input.payment_method_id);
        ::app::model::payment_methods::CustomerIdValue customer{};
        customer.set_value(input.customer_id);
        response.set_customer_id(std::move(customer));
        ::app::model::payment_methods::PaymentMethodValue method{};
        method.set_value(
            input.record.get_type().empty() ? std::string{"card"} : input.record.get_type()
        );
        response.set_payment_method(std::move(method));
        return ::app::model::payment_methods::CustomerDefaultPaymentMethodResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
