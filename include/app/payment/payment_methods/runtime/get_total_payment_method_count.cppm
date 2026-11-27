export module app_payment:payment_methods_runtime_get_total_payment_method_count;

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

class GetTotalPaymentMethodCountRuntime
{
public:
    explicit GetTotalPaymentMethodCountRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::TotalPaymentMethodCountResponse>
    get_total_payment_method_count(const core::behavior::CallContext& ctx) const
    {
        const auto input = GetTotalPaymentMethodCountData{ctx};
        m_context.metrics().record_flow("get_total_payment_method_count");
        m_context.metrics().counter("PAYMENT_METHOD_GET_TOTAL_PAYMENT_METHOD_COUNT").increment();
        std::int64_t count{0};
        for (const auto& record: m_context.repository().list_payment_methods()) {
            if (!input.customer_id.empty() && record.get_customer_id() != input.customer_id) {
                continue;
            }
            if (!input.merchant_id.empty() && record.get_merchant_id() != input.merchant_id) {
                continue;
            }
            ++count;
        }
        ::app::model::payment_methods::TotalPaymentMethodCountResponse response{};
        response.set_total_count(count);
        return ::app::model::payment_methods::TotalPaymentMethodCountResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
