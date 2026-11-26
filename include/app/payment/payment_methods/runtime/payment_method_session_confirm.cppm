export module app_payment:payment_methods_runtime_payment_method_session_confirm;

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

class PaymentMethodSessionConfirmRuntime
{
public:
    explicit PaymentMethodSessionConfirmRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<PaymentMethodSessionConfirmResult, core::error::AppError>
    payment_method_session_confirm(const PaymentMethodSessionConfirmInput& input) const
    {
        const auto& parsed = input.request;
        const auto& payload = input.payload;
        // Upstream core/payment_methods.rs:8256-8271 — storage_type follows the session's
        // intent only when customer_acceptance is present; PayThenVault forces volatile.
        std::string storage = "volatile";
        const auto stored =
            m_context.cache().get("payment_methods.session.storage:" + input.session_id);
        if (stored.hit) {
            storage = core::cache::value_as<std::string>(stored.value);
        }
        if (parsed.get_customer_acceptance()) {
            const auto& acceptance = parsed.get_customer_acceptance()->get_value();
            storage =
                acceptance.find("off_session") != std::string::npos ? "vault" : "payment_method";
        }
        const auto integration_type =
            m_context.config_store().get_value("payment_methods", "integration_type");
        if (integration_type && *integration_type == "pay_then_vault") {
            storage = "volatile";
        }
        const auto session_billing =
            m_context.cache().get("payment_methods.session.billing:" + input.session_id);
        if (session_billing.hit && input.body.find("billing") == std::string::npos) {
            const auto billing =
                m_support.decrypt_data(core::cache::value_as<std::string>(session_billing.value));
            m_context.cache().put(
                "payment_methods.confirm.billing:" + input.session_id,
                core::cache::to_cache_value(m_support.encrypt_data(billing))
            );
        }

        std::string vault{};
        if (storage == "vault") {
            const auto vault_response = m_support.call_connector(
                input.session.get_merchant_id(),
                "vault",
                input.session_id,
                payload
            );
            vault = vault_response.get_reference_id();
        }
        const auto zero_auth = m_support.call_connector(
            input.session.get_merchant_id(),
            "zero_auth",
            input.session_id,
            payload
        );
        if (zero_auth.get_status().empty() && vault.empty()) {
            const auto token_response = m_support.call_connector(
                input.session.get_merchant_id(),
                "tokenize",
                input.session_id,
                payload
            );
            vault = token_response.get_reference_id();
        } else if (vault.empty()) {
            vault = zero_auth.get_reference_id();
        }
        if (vault.empty()) {
            vault = m_support.issued_token(input.session_id);
        }
        const std::string payment_method_id = m_support.global_id("pm_", input.session_id);
        ::app::store::payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id(payment_method_id);
        record.set_merchant_id(input.session.get_merchant_id());
        record.set_type(parsed.get_payment_method_type().get_value());
        if (parsed.get_payment_method_subtype()) {
            record.set_type(parsed.get_payment_method_subtype()->get_value());
        }
        std::string last4{};
        const auto card_doc = serde::Ser::decode_generic("application/json", payload);
        if (card_doc) {
            if (const auto* object = std::get_if<interfaces::Value::Object>(&card_doc->get())) {
                for (const auto& [name, field]: *object) {
                    if (name != "card_number" && name != "number" && name != "card") {
                        continue;
                    }
                    if (const auto* text = std::get_if<std::string>(&field.get())) {
                        if (text->size() >= 4) {
                            last4 = text->substr(text->size() - 4);
                        }
                    }
                }
            }
        }
        record.set_last4(last4);
        record.set_vault_id(vault);
        record.set_status("active");
        record.set_created_at(m_support.now_value());
        auto repository = m_context.repository();
        auto stored_pm = repository.create_payment_method(std::move(record));
        if (!stored_pm) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::ALREADY_EXISTS, "payment method exists")
            };
        }
        m_context.cache().put(
            "payment_methods.card:" + payment_method_id,
            core::cache::to_cache_value(m_support.encrypt_data(payload))
        );
        const std::string single_use = m_support.issued_token(payment_method_id);
        m_context.cache().put(
            "payment_methods.confirm:" + payment_method_id,
            core::cache::to_cache_value(single_use),
            core::cache::Ttl{}.seconds(60)
        );
        m_context.cache().put(
            "payment_methods.token:" + single_use,
            core::cache::to_cache_value(payment_method_id),
            core::cache::Ttl{}.seconds(60)
        );
        const std::string parent_token =
            "token_" + m_support.global_id("", input.session_id).substr(0, 16);
        m_context.cache().put(
            "payment_methods.token:" + parent_token,
            core::cache::to_cache_value(payment_method_id),
            core::cache::Ttl{}.seconds(1'800)
        );
        m_context.cache().put(
            "payment_methods.session.associated:" + input.session_id,
            core::cache::to_cache_value(parent_token),
            core::cache::Ttl{}.seconds(1'800)
        );
        auto session = input.session;
        session.set_status("confirmed");
        repository.update_payment_method_session(session);
        m_support.publish_pm_event("payment_method_session.confirm", payment_method_id);
        m_context.metrics().record_flow("payment_method_session_confirm");
        m_context.metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_SESSION_CONFIRM").increment();
        ::app::model::payment_methods::PaymentMethodSessionResponse response{};
        ::app::model::payment_methods::GlobalPaymentMethodSessionIdValue id{};
        id.set_value(input.session_id);
        response.set_id(std::move(id));
        ::app::model::payment_methods::PrimitiveDateTimeValue expires_at{};
        expires_at.set_value(session.get_expires_at());
        response.set_expires_at(std::move(expires_at));
        ::app::model::payment_methods::StorageTypeValue storage_value{};
        storage_value.set_value(storage);
        response.set_storage_type(std::move(storage_value));
        return PaymentMethodSessionConfirmResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
