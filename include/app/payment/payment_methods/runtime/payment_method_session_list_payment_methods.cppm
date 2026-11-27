export module app_payment:payment_methods_runtime_payment_method_session_list_payment_methods;
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

class PaymentMethodSessionListPaymentMethodsRuntime
{
public:
    explicit PaymentMethodSessionListPaymentMethodsRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<
        ::app::model::payment_methods::PaymentMethodListResponseForSession>
    payment_method_session_list_payment_methods(
        const Dto& dto,
        const core::behavior::CallContext& ctx
    ) const
    {
        const auto input = PaymentMethodSessionListPaymentMethodsData{dto, ctx};
        const auto& session = input.session;
        m_context.logger().record_flow("payment_method_session_list_payment_methods");
        m_context.logger()
            .counter("PAYMENT_METHOD_PAYMENT_METHOD_SESSION_LIST_PAYMENT_METHODS")
            .increment();
        ::app::model::payment_methods::PaymentMethodListResponseForSession response{};
        std::vector<::app::model::payment_methods::CustomerPaymentMethodResponseItem> items{};
        for (const auto& record: m_context.repository().list_payment_methods()) {
            if (record.get_merchant_id() != session.get_merchant_id()) {
                continue;
            }
            if (!input.customer_id.empty() && record.get_customer_id() != input.customer_id) {
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
        response.set_customer_payment_methods(std::move(items));
        return ::app::model::payment_methods::PaymentMethodListResponseForSession{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
