export module app_handlers_payment_methods:runtime_list_customer_payment_method_api_client;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::payment_methods {

class ListCustomerPaymentMethodApiClientRuntime
{
public:
    explicit ListCustomerPaymentMethodApiClientRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListCustomerPaymentMethodApiClientResult, core::error::AppError>
    list_customer_payment_method_api_client(
        const ListCustomerPaymentMethodApiClientInput& input
    ) const
    {
        m_context.metrics().record_flow("list_customer_payment_method_api_client");
        m_context.metrics()
            .counter("PAYMENT_METHOD_LIST_CUSTOMER_PAYMENT_METHOD_API_CLIENT")
            .increment();
        app::model::payment_methods::ClientPaymentMethodsListResponse response{};
        std::vector<app::model::payment_methods::CustomerPaymentMethodForClient> items{};
        for (const auto& record: m_context.repository().list_payment_methods()) {
            if (!input.customer_id.empty() && record.get_customer_id() != input.customer_id) {
                continue;
            }
            app::model::payment_methods::CustomerPaymentMethodForClient item{};
            item.set_payment_token(
                record.get_vault_id().empty() ? record.get_payment_method_id()
                                              : record.get_vault_id()
            );
            app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method(std::move(method));
            app::model::payment_methods::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            item.set_created(std::move(created));
            items.push_back(std::move(item));
        }
        response.set_customer_payment_methods(std::move(items));
        return ListCustomerPaymentMethodApiClientResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
