export module app_handlers_payment_methods:runtime_get_payment_method_token_data;

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

class GetPaymentMethodTokenDataRuntime
{
public:
    explicit GetPaymentMethodTokenDataRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetPaymentMethodTokenDataResult, core::error::AppError>
    get_payment_method_token_data(const GetPaymentMethodTokenDataInput& input) const
    {
        const auto& record = input.record;
        const auto vault = m_support.call_connector(
            record.get_merchant_id(),
            "sync",
            input.payment_method_id,
            record.get_vault_id()
        );
        const std::string token =
            vault.get_reference_id().empty() ? record.get_vault_id() : vault.get_reference_id();
        m_context.metrics().record_flow("get_payment_method_token_data");
        m_context.metrics().counter("PAYMENT_METHOD_GET_PAYMENT_METHOD_TOKEN_DATA").increment();
        app::model::payment_methods::TokenDataResponse response{};
        app::model::payment_methods::GlobalPaymentMethodIdValue id{};
        id.set_value(input.payment_method_id);
        response.set_payment_method_id(std::move(id));
        app::model::payment_methods::TokenDataTypeValue type{};
        type.set_value(input.token_type);
        response.set_token_type(std::move(type));
        const auto encrypted =
            m_context.cache().get("payment_methods.card:" + input.payment_method_id);
        if (encrypted.hit) {
            const auto card_token =
                m_support.decrypt_data(core::cache::value_as<std::string>(encrypted.value));
            (void)card_token;
        }
        response.set_token_details(
            app::model::payment_methods::TokenDetailsResponse::NetworkTokenDetails
        );
        m_context.cache().put(
            "payment_methods.token:" + token,
            core::cache::to_cache_value(input.payment_method_id)
        );
        return GetPaymentMethodTokenDataResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
