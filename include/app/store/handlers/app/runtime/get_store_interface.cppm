export module app_store:app_runtime_get_store_interface;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :app_input;
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
    [[nodiscard]] std::expected<GetStoreInterfaceResult, core::error::AppError>
    get_store_interface(const GetStoreInterfaceInput& input) const
    {
        // Pick a storage interface from (storage_impl, event_handler) for the caller's tenant.
        const auto storage_impl = input.storage_impl;
        const auto event_handler = input.event_handler;
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
        const auto org_id = input.org_id;
        const auto merchant_id = input.merchant_id;
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

        GetStoreInterfaceResult result{};
        result.set_partition("app");
        result.set_op("get_store_interface");
        result.set_interface(interface_name);
        result.set_storage_impl(storage_impl);
        result.set_event_handler(event_handler);
        result.set_org_id(org_id);
        result.set_merchant_id(merchant_id);
        result.set_tenant_known(tenant_known);
        result.set_key_manager_configured(key_manager_configured);
        result.set_mtls_cert_configured(mtls_cert_configured);
        result.set_mtls_ca_configured(mtls_ca_configured);
        result.set_event_id(event_id);
        return result;
    }

private:
    ::app::context::AppContext& m_context;
    AppRuntimeSupport m_support;
};

} // namespace app::handlers::app
