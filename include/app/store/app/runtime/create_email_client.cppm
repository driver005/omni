export module app_store:app_runtime_create_email_client;

import std;
import core_behavior;
import interfaces;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :app_result;
import :app_runtime_common;

export namespace app::handlers::app {

class CreateEmailClientRuntime
{
public:
    explicit CreateEmailClientRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream create_email_client() is a free helper (app.rs:477-495), no HTTP auth.
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create_email_client(
        const serde::Value& payload,
        const core::behavior::CallContext& ctx
    ) const
    {
        // Upstream create_email_client(): choose from EmailClientConfigs
        // (ses / smtp / no_email_client). Credentials are secrets and must not
        // be returned to the caller in the clear.
        const auto configured_client = m_support.field_string(payload, "client_config");
        auto client_type = configured_client.value_or("no_email_client");
        if (client_type != "ses" && client_type != "smtp" && client_type != "no_email_client") {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "email client_config must be ses, smtp or no_email_client"
            )};
        }
        // The upstream config-driven default when the caller omits client_config.
        if (!configured_client) {
            client_type = m_context.config_store()
                              .get_value("email_client", "default_config")
                              .value_or(client_type);
        }

        const auto query_proxy = ctx.query.find("proxy_url");
        const auto proxy_url = query_proxy != ctx.query.end() && !query_proxy->second.empty()
            ? query_proxy->second
            : m_context.config_store().get_value("email_client", "proxy_url").value_or("");
        const auto client_id = m_context.ids().key_id();

        // Deterministic substitute for upstream secret fetch (fetch_raw_secrets): a caller may
        // pass a key-store id; the active value is resolved through the injected KeyStore and
        // never echoed. The literal field is only a fallback for callers without a managed key.
        ::app::crypto::SecretString secret{
            m_support.field_string(payload, "secret").value_or("")
        };
        bool key_configured{false};
        const auto secret_id = m_support.field_string(payload, "secret_id").value_or("");
        if (!secret_id.empty()) {
            const auto managed = m_context.key_store().get("email_client", secret_id);
            if (managed) {
                secret = *managed;
                key_configured = true;
            }
        }

        // Ephemeral registry of freshly constructed clients, keyed by instance id. Only the
        // selection is cached; the raw request body and the secret are never cached.
        m_context.cache().put(
            "app.email_client:" + client_id,
            core::cache::to_cache_value(client_type)
        );

        // Event-side substitute for constructing the upstream Box<dyn EmailService>: record the
        // deterministic construction as a persisted event instead of returning a fake client.
        serde::Value::Object event_body{};
        event_body["client_id"] = serde::Value{client_id};
        event_body["client_config"] = serde::Value{client_type};
        const auto event_payload =
            m_support.serialize_document(serde::Value{std::move(event_body)});
        const auto event_id = m_context.events().publish("email_client.created", event_payload);

        m_context.metrics().record_flow("CreateEmailClient");
        m_context.metrics().counter("EMAIL_CLIENT_CREATED").increment();

        serde::Value::Object document{};
        document.insert(
            std::string{create_email_client_wire_fields()[0]},
            serde::Value{std::string{"app"}}
        );
        document.insert(
            std::string{create_email_client_wire_fields()[1]},
            serde::Value{std::string{"create_email_client"}}
        );
        document.insert(std::string{create_email_client_wire_fields()[2]}, serde::Value{client_id});
        document.insert(
            std::string{create_email_client_wire_fields()[3]},
            serde::Value{client_type}
        );
        document.insert(std::string{create_email_client_wire_fields()[4]}, serde::Value{proxy_url});
        document.insert(
            std::string{create_email_client_wire_fields()[5]},
            serde::Value{secret.masked_as(::app::crypto::SecretFieldKind::ConnectorCredential)}
        );
        document.insert(
            std::string{create_email_client_wire_fields()[6]},
            serde::Value{key_configured}
        );
        document.insert(std::string{create_email_client_wire_fields()[7]}, serde::Value{event_id});
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    AppRuntimeSupport m_support;
};

} // namespace app::handlers::app
