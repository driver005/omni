export module app_payment:payment_methods_runtime_payment_methods_session_create;
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

class PaymentMethodsSessionCreateRuntime
{
public:
    explicit PaymentMethodsSessionCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<
        ::app::model::payment_methods::PaymentMethodSessionResponse>
    payment_methods_session_create(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentMethodsSessionCreateData{dto, ctx};
        const auto& parsed = input.request;
        const std::string session_id =
            m_support.global_id("pms_", input.merchant_id + input.org_id);
        const std::string client_secret = m_support.issued_token(session_id);
        const std::int64_t expires =
            std::stoll(m_support.now_value()) +
            static_cast<std::int64_t>(parsed.get_expires_in().value_or(1'800u));
        ::app::store::payment_method_sessions::PaymentMethodSessionRecord record{};
        record.set_session_id(session_id);
        record.set_merchant_id(input.merchant_id.empty() ? input.org_id : input.merchant_id);
        record.set_client_secret(m_support.encrypt_data(client_secret));
        record.set_status("active");
        record.set_created_at(m_support.now_value());
        record.set_expires_at(std::to_string(expires));
        auto stored = m_context.repository().create_payment_method_session(std::move(record));
        if (!stored) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::ALREADY_EXISTS, "session already exists")
            };
        }
        m_support.remember_client_secret(session_id, client_secret);
        m_context.cache().put(
            "payment_methods.session.storage:" + session_id,
            core::cache::to_cache_value(parsed.get_storage_type().get_value())
        );
        if (parsed.get_billing()) {
            m_context.cache().put(
                "payment_methods.session.billing:" + session_id,
                core::cache::to_cache_value(
                    m_support.encrypt_data(serde::Ser::encode_json(*parsed.get_billing()))
                )
            );
        }
        if (parsed.get_tokenization_data()) {
            m_context.cache().put(
                "payment_methods.session.tokenization:" + session_id,
                core::cache::to_cache_value(
                    m_support.encrypt_data(serde::Ser::encode_json(*parsed.get_tokenization_data()))
                )
            );
        }
        m_support.publish_pm_event("payment_method_session.create", session_id);
        m_context.logger().record_flow("payment_methods_session_create");
        m_context.logger().counter("PAYMENT_METHOD_PAYMENT_METHODS_SESSION_CREATE").increment();
        ::app::model::payment_methods::PaymentMethodSessionResponse response{};
        ::app::model::payment_methods::GlobalPaymentMethodSessionIdValue id{};
        id.set_value(session_id);
        response.set_id(std::move(id));
        response.set_client_secret(client_secret);
        ::app::model::payment_methods::PrimitiveDateTimeValue expires_at{};
        expires_at.set_value(std::to_string(expires));
        response.set_expires_at(std::move(expires_at));
        response.set_storage_type(parsed.get_storage_type());
        if (parsed.get_customer_id()) {
            response.set_customer_id(*parsed.get_customer_id());
        }
        if (parsed.get_billing()) {
            response.set_billing(*parsed.get_billing());
        }
        if (parsed.get_return_url()) {
            response.set_return_url(*parsed.get_return_url());
        }
        response.set_keep_alive(parsed.get_keep_alive().value_or(false));
        return ::app::model::payment_methods::PaymentMethodSessionResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
