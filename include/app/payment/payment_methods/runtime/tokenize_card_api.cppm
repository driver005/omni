export module app_payment:payment_methods_runtime_tokenize_card_api;

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

class TokenizeCardApiRuntime
{
public:
    explicit TokenizeCardApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::payment_methods::CardNetworkTokenizeResponse, core::error::AppError>
        tokenize_card_api(const TokenizeCardApiInput& input) const
    {
        const auto payload = serde::Ser::encode_json(input.request);
        const auto vault =
            m_support
                .call_connector(input.merchant_id, "tokenize", m_context.ids().uuid_v4(), payload);
        const std::string token = vault.get_reference_id().empty()
                                      ? m_support.issued_token(m_context.ids().uuid_v4())
                                      : vault.get_reference_id();
        m_context.cache().put(
            "payment_methods.card:" + token,
            core::cache::to_cache_value(m_support.encrypt_data(payload))
        );
        m_context.metrics().record_flow("tokenize_card_api");
        m_context.metrics().counter("PAYMENT_METHOD_TOKENIZE_CARD_API").increment();
        ::app::model::payment_methods::CardNetworkTokenizeResponse response{};
        response.set_card_tokenized(true);
        ::app::model::payment_methods::PaymentMethodResponse card_response =
            m_support.response_from_record(
                [&]()
                {
                    ::app::store::payment_methods::PaymentMethodRecord record{};
                    record.set_merchant_id(input.merchant_id);
                    record.set_payment_method_id(token);
                    record.set_vault_id(token);
                    return record;
                }()
            );
        card_response.set_client_secret(token);
        response.set_payment_method_response(std::move(card_response));
        return ::app::model::payment_methods::CardNetworkTokenizeResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
