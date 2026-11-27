export module app_payment:payment_methods_runtime_create_payment_method_intent_api;

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

class CreatePaymentMethodIntentApiRuntime
{
public:
    explicit CreatePaymentMethodIntentApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::payment_methods::PaymentMethodResponse, core::error::AppError>
        create_payment_method_intent_api(const CreatePaymentMethodIntentApiInput& input) const
    {
        const std::string& customer_id = input.request.get_customer_id().get_value();
        if (!customer_id.empty() && !m_support.customer_exists(customer_id)) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            )};
        }
        const std::string payment_method_id =
            m_support.global_id("pmi_", input.merchant_id + customer_id);
        ::app::store::payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id(payment_method_id);
        record.set_merchant_id(input.merchant_id);
        record.set_customer_id(customer_id);
        record.set_status("requires_confirmation");
        record.set_created_at(m_support.now_value());
        auto stored = m_context.repository().create_payment_method(std::move(record));
        if (!stored) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::ALREADY_EXISTS,
                "payment method intent exists"
            )};
        }
        const std::string client_secret = m_support.issued_token(payment_method_id);
        m_support.remember_client_secret(payment_method_id, client_secret);
        m_support.publish_pm_event("payment_method_intent.create", payment_method_id);
        m_context.metrics().record_flow("create_payment_method_intent_api");
        m_context.metrics().counter("PAYMENT_METHOD_CREATE_PAYMENT_METHOD_INTENT_API").increment();
        auto response = m_support.response_from_record(*stored);
        if (input.request.get_metadata()) {
            response.set_metadata(*input.request.get_metadata());
        }
        response.set_client_secret(client_secret);
        return ::app::model::payment_methods::PaymentMethodResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
