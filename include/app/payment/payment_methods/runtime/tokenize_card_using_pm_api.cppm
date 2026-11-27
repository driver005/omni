export module app_payment:payment_methods_runtime_tokenize_card_using_pm_api;
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

class TokenizeCardUsingPmApiRuntime
{
public:
    explicit TokenizeCardUsingPmApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payment_methods::TokenizedCardValue1>
    tokenize_card_using_pm_api(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = TokenizeCardUsingPmApiData{dto, ctx};
        const auto& record = input.record;
        const auto vault = m_support.call_connector(
            record.get_merchant_id(),
            "tokenize",
            input.payment_method_id,
            serde::Ser::encode_json(input.request)
        );
        const std::string token = vault.get_reference_id().empty()
                                      ? m_support.issued_token(input.payment_method_id)
                                      : vault.get_reference_id();
        m_context.cache().put(
            "payment_methods.token:" + token,
            core::cache::to_cache_value(input.payment_method_id)
        );
        m_context.logger().record_flow("tokenize_card_using_pm_api");
        m_context.logger().counter("PAYMENT_METHOD_TOKENIZE_CARD_USING_PM_API").increment();
        ::app::model::payment_methods::TokenizedCardValue1 response{};
        response.set_card_number("************" + record.get_last4());
        response.set_exp_year("");
        response.set_exp_month("");
        response.set_card_last_four(record.get_last4());
        response.set_card_token(token);
        return ::app::model::payment_methods::TokenizedCardValue1{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
