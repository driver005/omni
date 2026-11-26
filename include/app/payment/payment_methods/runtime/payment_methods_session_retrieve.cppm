export module app_payment:payment_methods_runtime_payment_methods_session_retrieve;

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

class PaymentMethodsSessionRetrieveRuntime
{
public:
    explicit PaymentMethodsSessionRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<PaymentMethodsSessionRetrieveResult, core::error::AppError>
    payment_methods_session_retrieve(const PaymentMethodsSessionRetrieveInput& input) const
    {
        const auto& record = input.record;
        m_context.metrics().record_flow("payment_methods_session_retrieve");
        m_context.metrics().counter("PAYMENT_METHOD_PAYMENT_METHODS_SESSION_RETRIEVE").increment();
        ::app::model::payment_methods::PaymentMethodSessionResponse response{};
        ::app::model::payment_methods::GlobalPaymentMethodSessionIdValue id{};
        id.set_value(input.session_id);
        response.set_id(std::move(id));
        response.set_client_secret(m_support.decrypt_data(record.get_client_secret()));
        ::app::model::payment_methods::PrimitiveDateTimeValue expires_at{};
        expires_at.set_value(record.get_expires_at());
        response.set_expires_at(std::move(expires_at));
        ::app::model::payment_methods::StorageTypeValue storage{};
        const auto stored_storage =
            m_context.cache().get("payment_methods.session.storage:" + input.session_id);
        storage.set_value(
            stored_storage.hit ? core::cache::value_as<std::string>(stored_storage.value)
                               : std::string{"payment_methods"}
        );
        response.set_storage_type(std::move(storage));
        return PaymentMethodsSessionRetrieveResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
