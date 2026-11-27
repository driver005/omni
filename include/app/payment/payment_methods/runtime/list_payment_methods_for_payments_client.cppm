export module app_payment:payment_methods_runtime_list_payment_methods_for_payments_client;

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

class ListPaymentMethodsForPaymentsClientRuntime
{
public:
    explicit ListPaymentMethodsForPaymentsClientRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::payments::PaymentsClientMethodsResponse, core::error::AppError>
        list_payment_methods_for_payments_client(
            const ListPaymentMethodsForPaymentsClientInput& input
        ) const
    {
        m_context.metrics().record_flow("list_payment_methods_for_payments_client");
        m_context.metrics()
            .counter("PAYMENT_METHOD_LIST_PAYMENT_METHODS_FOR_PAYMENTS_CLIENT")
            .increment();
        ::app::model::payment_methods::PaymentMethodListResponseForSession response{};
        std::vector<::app::model::payment_methods::CustomerPaymentMethodResponseItem> items{};
        for (const auto& record: m_context.repository().list_payment_methods()) {
            if (!input.merchant_id.empty() && record.get_merchant_id() != input.merchant_id) {
                continue;
            }
            ::app::model::payment_methods::CustomerPaymentMethodResponseItem item{};
            item.set_payment_method_token(
                record.get_vault_id().empty() ? record.get_payment_method_id()
                                              : record.get_vault_id()
            );
            ::app::model::payment_methods::GlobalCustomerIdValue customer{};
            customer.set_value(record.get_customer_id());
            item.set_customer_id(std::move(customer));
            ::app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method_type(std::move(method));
            ::app::model::payment_methods::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            item.set_created(created);
            ::app::model::payment_methods::PrimitiveDateTimeValue last_used{};
            last_used.set_value(record.get_created_at());
            item.set_last_used_at(std::move(last_used));
            items.push_back(std::move(item));
        }
        response.set_customer_payment_methods(items);
        ::app::model::payments::PaymentsClientMethodsResponse client{};
        client.set_count(items.size());
        std::vector<::app::model::payments::PaymentsClientMethod> methods{};
        methods.reserve(items.size());
        for (const auto& item: items) {
            ::app::model::payments::PaymentsClientMethod method{};
            const auto& type = item.get_payment_method_type();
            method.set_payment_method(type.get_value());
            method.set_payment_method_type(type.get_value());
            method.set_enabled(true);
            methods.push_back(std::move(method));
        }
        client.set_data(std::move(methods));
        return client;
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
