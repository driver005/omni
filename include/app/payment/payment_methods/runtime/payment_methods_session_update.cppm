export module app_payment:payment_methods_runtime_payment_methods_session_update;

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

class PaymentMethodsSessionUpdateRuntime
{
public:
    explicit PaymentMethodsSessionUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::payment_methods::PaymentMethodSessionResponse, core::error::AppError>
        payment_methods_session_update(const PaymentMethodsSessionUpdateInput& input) const
    {
        auto record = input.record;
        const auto& parsed = input.request;
        if (parsed.get_billing()) {
            m_context.cache().put(
                "payment_methods.session.billing:" + input.session_id,
                core::cache::to_cache_value(
                    m_support.encrypt_data(serde::Ser::encode_json(*parsed.get_billing()))
                )
            );
        }
        if (parsed.get_tokenization_data()) {
            m_context.cache().put(
                "payment_methods.session.tokenization:" + input.session_id,
                core::cache::to_cache_value(
                    m_support.encrypt_data(serde::Ser::encode_json(*parsed.get_tokenization_data()))
                )
            );
        }
        const std::string secret = m_support.issued_token(input.session_id);
        record.set_client_secret(m_support.encrypt_data(secret));
        if (!m_context.repository().update_payment_method_session(record)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INTERNAL_ERROR, "persistence failed")
            };
        }
        m_support.remember_client_secret(input.session_id, secret);
        m_context.metrics().record_flow("payment_methods_session_update");
        m_context.metrics().counter("PAYMENT_METHOD_PAYMENT_METHODS_SESSION_UPDATE").increment();
        ::app::model::payment_methods::PaymentMethodSessionResponse response{};
        ::app::model::payment_methods::GlobalPaymentMethodSessionIdValue id{};
        id.set_value(input.session_id);
        response.set_id(std::move(id));
        response.set_client_secret(secret);
        ::app::model::payment_methods::PrimitiveDateTimeValue expires_at{};
        expires_at.set_value(record.get_expires_at());
        response.set_expires_at(std::move(expires_at));
        ::app::model::payment_methods::StorageTypeValue storage{};
        storage.set_value("payment_methods");
        response.set_storage_type(std::move(storage));
        return ::app::model::payment_methods::PaymentMethodSessionResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
