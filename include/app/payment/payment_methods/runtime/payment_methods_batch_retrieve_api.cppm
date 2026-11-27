export module app_payment:payment_methods_runtime_payment_methods_batch_retrieve_api;
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

class PaymentMethodsBatchRetrieveApiRuntime
{
public:
    explicit PaymentMethodsBatchRetrieveApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<
        std::vector<::app::model::payment_methods::PaymentMethodsBatchRetrieveResponse>>
    payment_methods_batch_retrieve_api(const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentMethodsBatchRetrieveApiData{ctx};
        m_context.logger().record_flow("payment_methods_batch_retrieve_api");
        m_context.logger().counter("PAYMENT_METHOD_PAYMENT_METHODS_BATCH_RETRIEVE_API").increment();
        auto repository = m_context.repository();
        std::vector<::app::model::payment_methods::PaymentMethodsBatchRetrieveResponse> responses{};
        for (const auto& [payment_method_id, line_number]: input.rows) {
            ::app::model::payment_methods::PaymentMethodsBatchRetrieveResponse response{};
            response.set_payment_method_id(payment_method_id);
            response.set_line_number(line_number);
            const auto record = repository.find_payment_method_by_string(payment_method_id);
            if (!record) {
                response.set_error_message(std::string{"Payment method not found"});
            } else {
                if (!record->get_type().empty()) {
                    ::app::model::payment_methods::PaymentMethodValue method{};
                    method.set_value(record->get_type());
                    response.set_payment_method_type(std::move(method));
                }
            }
            responses.push_back(std::move(response));
        }
        return std::vector<::app::model::payment_methods::PaymentMethodsBatchRetrieveResponse>{
            std::move(responses)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
