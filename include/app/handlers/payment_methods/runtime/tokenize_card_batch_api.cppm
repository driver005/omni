export module app_handlers_payment_methods:runtime_tokenize_card_batch_api;

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

class TokenizeCardBatchApiRuntime
{
public:
    explicit TokenizeCardBatchApiRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<TokenizeCardBatchApiResult, core::error::AppError>
    tokenize_card_batch_api(const TokenizeCardBatchApiInput& input) const
    {
        std::vector<app::model::payment_methods::TokenizedCardValue1> results{};
        for (const auto& payload: input.payloads) {
            const auto vault = m_support.call_connector(
                input.merchant_id,
                "tokenize",
                m_context.ids().uuid_v4(),
                payload
            );
            const std::string token = vault.get_reference_id().empty()
                                          ? m_support.issued_token(m_context.ids().uuid_v4())
                                          : vault.get_reference_id();
            m_context.cache().put(
                "payment_methods.card:" + token,
                core::cache::to_cache_value(m_support.encrypt_data(payload))
            );
            app::model::payment_methods::TokenizedCardValue1 item{};
            item.set_card_number("************");
            item.set_exp_year("");
            item.set_exp_month("");
            item.set_card_token(token);
            results.push_back(std::move(item));
        }
        m_context.metrics().record_flow("tokenize_card_batch_api");
        m_context.metrics().counter("PAYMENT_METHOD_TOKENIZE_CARD_BATCH_API").increment();
        return TokenizeCardBatchApiResult{std::move(results)};
    }

private:
    app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
