export module app_payment:payment_methods_runtime_list_customer_payment_method_api;

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

class ListCustomerPaymentMethodApiRuntime
{
public:
    explicit ListCustomerPaymentMethodApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::CustomerPaymentMethodsListResponse>
    list_customer_payment_method_api(const core::behavior::CallContext& ctx) const
    {
        const auto input = ListCustomerPaymentMethodApiData{ctx};
        m_context.metrics().record_flow("list_customer_payment_method_api");
        m_context.metrics().counter("PAYMENT_METHOD_LIST_CUSTOMER_PAYMENT_METHOD_API").increment();
        ::app::model::payment_methods::CustomerPaymentMethodsListResponse response{};
        std::vector<::app::model::payment_methods::CustomerPaymentMethod> items{};
        const auto default_key = "payment_methods.default:" + input.customer_id;
        const auto default_hit = m_context.cache().get(default_key);
        const std::string default_id =
            default_hit.hit ? core::cache::value_as<std::string>(default_hit.value) : std::string{};
        for (const auto& record: m_context.repository().list_payment_methods()) {
            if (record.get_customer_id() != input.customer_id) {
                continue;
            }
            if (!input.merchant_id.empty() && record.get_merchant_id() != input.merchant_id) {
                continue;
            }
            ::app::model::payment_methods::CustomerPaymentMethod item{};
            item.set_payment_token(
                record.get_vault_id().empty() ? record.get_payment_method_id()
                                              : record.get_vault_id()
            );
            item.set_payment_method_id(record.get_payment_method_id());
            ::app::model::payment_methods::CustomerIdValue customer{};
            customer.set_value(input.customer_id);
            item.set_customer_id(std::move(customer));
            ::app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method(std::move(method));
            item.set_default_payment_method_set(record.get_payment_method_id() == default_id);
            ::app::model::payment_methods::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            item.set_created(std::move(created));
            items.push_back(std::move(item));
        }
        response.set_customer_payment_methods(std::move(items));
        return ::app::model::payment_methods::CustomerPaymentMethodsListResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
