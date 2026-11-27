export module app_store:app_runtime_get_store_interface;

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

class GetStoreInterfaceRuntime
{
public:
    explicit GetStoreInterfaceRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream AppState::get_store_interface() is an async constructor (app.rs:648-727).
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_store_interface(const core::behavior::CallContext& ctx) const
    {
        // Pick a storage interface from (storage_impl, event_handler) for the caller's tenant.
        const auto storage_impl = m_support.query_or(ctx, "storage_impl", "postgresql");
        const auto event_handler = m_support.query_or(ctx, "event_handler", "logs");
        if (!m_support.known_storage_impl(storage_impl)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "storage_impl must be postgresql, postgresql_test or mock"
            )};
        }
        if (event_handler != "kafka" && event_handler != "logs") {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "event_handler must be kafka or logs"
            )};
        }

        const std::string interface_name =
            storage_impl == "mock" ? "mock_db"
                                   : (event_handler == "kafka" ? "kafka_store" : "postgres_store");
        const auto& org_id = ctx.org_id;
        const auto& merchant_id = ctx.merchant_id;
        bool tenant_known = true;
        if (!merchant_id.empty()) {
            try {
                tenant_known =
                    m_context.repository().find_merchant_account_by_string(merchant_id).has_value();
            } catch (...) {
                tenant_known = false;
            }
        }

        // Deterministic substitutes for upstream startup state that has no HTTP wire shape:
        //  * KeyManagerState construction -> active master key presence in the KeyStore.
        //  * mTLS cert/CA fields -> presence of resolved secrets, never their raw value.
        //  * KafkaStore/Logs event handler -> a persisted event record.
        const bool key_manager_configured =
            m_context.key_store().get("key_manager", "master_key").has_value();
        const bool mtls_cert_configured =
            m_context.secret_resolver().resolve("key_manager", "mtls_cert").has_value();
        const bool mtls_ca_configured =
            m_context.secret_resolver().resolve("key_manager", "mtls_ca").has_value();
        serde::Value::Object event_body{};
        event_body["interface"] = serde::Value{interface_name};
        event_body["event_handler"] = serde::Value{event_handler};
        const auto event_payload =
            m_support.serialize_document(serde::Value{std::move(event_body)});
        const auto event_id = m_context.events().publish("store_interface.resolved", event_payload);

        m_context.metrics().record_flow("GetStoreInterface");
        m_context.metrics().counter("STORE_INTERFACE_RESOLVED").increment();

        serde::Value::Object document{};
        document.insert(
            std::string{get_store_interface_wire_fields()[0]},
            serde::Value{std::string{"app"}}
        );
        document.insert(
            std::string{get_store_interface_wire_fields()[1]},
            serde::Value{std::string{"get_store_interface"}}
        );
        document.insert(
            std::string{get_store_interface_wire_fields()[2]},
            serde::Value{interface_name}
        );
        document.insert(
            std::string{get_store_interface_wire_fields()[3]},
            serde::Value{storage_impl}
        );
        document.insert(
            std::string{get_store_interface_wire_fields()[4]},
            serde::Value{event_handler}
        );
        document.insert(std::string{get_store_interface_wire_fields()[5]}, serde::Value{org_id});
        document.insert(
            std::string{get_store_interface_wire_fields()[6]},
            serde::Value{merchant_id}
        );
        document.insert(
            std::string{get_store_interface_wire_fields()[7]},
            serde::Value{tenant_known}
        );
        document.insert(
            std::string{get_store_interface_wire_fields()[8]},
            serde::Value{key_manager_configured}
        );
        document.insert(
            std::string{get_store_interface_wire_fields()[9]},
            serde::Value{mtls_cert_configured}
        );
        document.insert(
            std::string{get_store_interface_wire_fields()[10]},
            serde::Value{mtls_ca_configured}
        );
        document.insert(std::string{get_store_interface_wire_fields()[11]}, serde::Value{event_id});
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    AppRuntimeSupport m_support;
};

} // namespace app::handlers::app
