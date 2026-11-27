export module app_user:admin_runtime_retrieve_connector_webhook;

import std;
import core_behavior;
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

import :admin_input;
import :admin_result;
import :admin_runtime_common;

export namespace app::handlers::admin {

class RetrieveConnectorWebhookRuntime
{
public:
    explicit RetrieveConnectorWebhookRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::admin::WebhookDetailsResponse>
    fetch(const ::core::behavior::CallContext& ctx) const
    {
        const auto found =
            m_context.repository().find_connector_webhook_by_string(ctx_string(ctx,"connector_id"));
        if (!found.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "Merchant connector account does not exist in our records"
            )};
        }
        const std::string decrypted{::app::crypto::AesWrap{}.decrypt(
            found->get_secret(),
            m_support.connector_encryption_key(found->get_merchant_id(), ctx_string(ctx,"connector_id"))
        )};
        ::app::dto::admin::WebhookDetailsResponse response;
        if (!found->get_event_type().empty()) {
            response.set_webhook_version(found->get_event_type());
        }
        if (!found->get_scope().empty()) {
            response.set_webhook_url(found->get_scope());
        }
        if (!decrypted.empty()) {
            response.set_webhook_password(decrypted);
        }

        m_context.metrics().record_flow("retrieve_connector_webhook");
        m_context.metrics().counter("admin.retrieve_connector_webhook").increment();
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    AdminRuntimeSupport m_support;
};

} // namespace app::handlers::admin
