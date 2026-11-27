export module app_payment:payment_methods_runtime_payment_method_retrieve_olap_api;

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

class PaymentMethodRetrieveOlapApiRuntime
{
public:
    explicit PaymentMethodRetrieveOlapApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::PaymentMethodDetailsResponse>
        payment_method_retrieve_olap_api(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentMethodRetrieveOlapApiData{dto, ctx};
        const auto& record = input.record;
        m_context.metrics().record_flow("payment_method_retrieve_olap_api");
        m_context.metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_RETRIEVE_OLAP_API").increment();
        ::app::model::payment_methods::PaymentMethodDetailsResponse response{};
        ::app::model::payment_methods::GlobalPaymentMethodIdValue id{};
        id.set_value(record.get_payment_method_id());
        response.set_id(std::move(id));
        ::app::model::payment_methods::MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        response.set_merchant_id(std::move(merchant));
        if (!record.get_customer_id().empty()) {
            ::app::model::payment_methods::GlobalCustomerIdValue customer{};
            customer.set_value(record.get_customer_id());
            response.set_customer_id(std::move(customer));
        }
        ::app::model::payment_methods::PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        response.set_created(std::move(created));
        ::app::model::payment_methods::StorageTypeValue storage{};
        storage.set_value("payment_methods");
        response.set_storage_type(std::move(storage));
        return ::app::model::payment_methods::PaymentMethodDetailsResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
