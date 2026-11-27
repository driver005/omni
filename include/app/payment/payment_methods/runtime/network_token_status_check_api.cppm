export module app_payment:payment_methods_runtime_network_token_status_check_api;
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

class NetworkTokenStatusCheckApiRuntime
{
public:
    explicit NetworkTokenStatusCheckApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<
        ::app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse>
    network_token_status_check_api(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = NetworkTokenStatusCheckApiData{dto, ctx};
        const auto& record = input.record;
        const auto status_response = m_support.call_connector(
            record.get_merchant_id(),
            "network_token_status",
            input.payment_method_id,
            record.get_vault_id()
        );
        const std::string status = status_response.get_status().empty()
                                       ? record.get_status()
                                       : status_response.get_status();
        m_context.logger().record_flow("network_token_status_check_api");
        m_context.logger().counter("PAYMENT_METHOD_NETWORK_TOKEN_STATUS_CHECK_API").increment();
        ::app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse response{};
        ::app::model::payment_methods::TokenStatusValue value{};
        value.set_value(status.empty() ? std::string{"active"} : status);
        response.set_status(std::move(value));
        ::app::model::payment_methods::GlobalPaymentMethodIdValue id{};
        id.set_value(input.payment_method_id);
        response.set_payment_method_id(std::move(id));
        return ::app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
