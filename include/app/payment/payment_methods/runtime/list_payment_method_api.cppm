export module app_payment:payment_methods_runtime_list_payment_method_api;
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

class ListPaymentMethodApiRuntime
{
public:
    explicit ListPaymentMethodApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::PaymentMethodListResponse>
    list_payment_method_api(const core::behavior::CallContext& ctx) const
    {
        const auto input = ListPaymentMethodApiData{ctx};
        m_context.logger().record_flow("list_payment_method_api");
        m_context.logger().counter("PAYMENT_METHOD_LIST_PAYMENT_METHOD_API").increment();
        ::app::model::payment_methods::PaymentMethodListResponse response{};
        std::vector<::app::model::payment_methods::ResponsePaymentMethodsEnabled> enabled{};
        for (const auto& record: m_context.repository().list_payment_methods()) {
            if (enabled.size() >= input.limit) {
                break;
            }
            if (!input.merchant_id.empty() && record.get_merchant_id() != input.merchant_id) {
                continue;
            }
            ::app::model::payment_methods::ResponsePaymentMethodsEnabled item{};
            ::app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method(std::move(method));
            enabled.push_back(std::move(item));
        }
        response.set_payment_methods(std::move(enabled));
        return ::app::model::payment_methods::PaymentMethodListResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
