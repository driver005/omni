export module app_handlers_admin:runtime_connector_webhook_register;

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
import app_locking;
import app_events;
import app_keystore;
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::admin {

class ConnectorWebhookRegisterRuntime
{
public:
    explicit ConnectorWebhookRegisterRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ConnectorWebhookRegisterResult, core::error::AppError>
    register_webhook(const ConnectorWebhookRegisterInput& input) const
    {
        // The connector-hosted webhook registration API is external; the local
        // ConnectorWebhookRecord + encrypted secret + delivery enqueue is the
        // in-process substitute while preserving the request/response wire shape.
        std::string connector_name{input.connector_id};
        const auto connector =
            m_context.repository().find_merchant_connector_by_string(input.connector_id);
        if (connector.has_value() && !connector->get_connector_name().empty()) {
            connector_name = connector->get_connector_name();
        }
        if (const auto duplicate = m_support.acquire_operation_lock(
                "connector_webhook_register",
                input.connector_id
            )) {
            return std::unexpected{*duplicate};
        }
        const app::crypto::SecretString password{input.request.get_webhook_password().value_or("")};
        app::store::connector_webhooks::ConnectorWebhookRecord record;
        record.set_connector_webhook_id(std::string{input.connector_id});
        record.set_connector(connector_name);
        record.set_merchant_id(std::string{input.merchant_id});
        record.set_event_type(input.request.get_webhook_version().value_or(""));
        record.set_scope(input.request.get_webhook_url().value_or(""));
        record.set_secret(
            app::crypto::AesWrap{}.encrypt(
                password.value(),
                m_support.connector_encryption_key(input.merchant_id, input.connector_id)
            )
        );
        record.set_status("active");
        const auto saved = m_context.repository().save_connector_webhook(std::move(record));
        m_support.release_operation_lock("connector_webhook_register", input.connector_id);

        app::dto::admin::WebhookDetailsResponse response;
        if (!saved.get_event_type().empty()) {
            response.set_webhook_version(saved.get_event_type());
        }
        if (!saved.get_scope().empty()) {
            response.set_webhook_url(saved.get_scope());
        }
        const std::string decrypted_password{app::crypto::AesWrap{}.decrypt(
            saved.get_secret(),
            m_support.connector_encryption_key(saved.get_merchant_id(), input.connector_id)
        )};
        if (!decrypted_password.empty()) {
            response.set_webhook_password(decrypted_password);
        }
        response.set_webhook_username(input.request.get_webhook_username());
        response.set_payment_created_enabled(input.request.get_payment_created_enabled());
        response.set_payment_succeeded_enabled(input.request.get_payment_succeeded_enabled());
        response.set_payment_failed_enabled(input.request.get_payment_failed_enabled());
        response.set_payment_statuses_enabled(input.request.get_payment_statuses_enabled());
        response.set_refund_statuses_enabled(input.request.get_refund_statuses_enabled());
        response.set_payout_statuses_enabled(input.request.get_payout_statuses_enabled());
        response.set_dispute_statuses_enabled(input.request.get_dispute_statuses_enabled());
        response.set_mandate_statuses_enabled(input.request.get_mandate_statuses_enabled());
        response.set_invoice_statuses_enabled(input.request.get_invoice_statuses_enabled());

        const std::string event_id{m_context.events().publish(
            "connector_webhook.registered",
            std::string{serde::Ser::encode_json(response)}
        )};
        if (!saved.get_scope().empty()) {
            m_context.webhook_delivery().enqueue(event_id, saved.get_scope());
        }

        m_context.metrics().record_flow("connector_webhook_register");
        m_context.metrics().counter("admin.connector_webhook_register").increment();
        return ConnectorWebhookRegisterResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
