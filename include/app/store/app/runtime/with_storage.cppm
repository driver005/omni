export module app_store:app_runtime_with_storage;

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

class WithStorageRuntime
{
public:
    explicit WithStorageRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream AppState::with_storage() is an async constructor (app.rs:497-643), no auth.
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    with_storage(const core::behavior::CallContext& ctx) const
    {
        const auto storage_impl = m_support.query_or(ctx, "storage_impl", "postgresql");
        const auto service_name = m_support.query_or(ctx, "service_name", "default");
        if (!m_support.known_storage_impl(storage_impl)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "storage_impl must be postgresql, postgresql_test or mock"
            )};
        }

        const bool storage_ok = m_support.storage_available();
        const auto session_id = m_context.ids().uuid_v4();

        // Deterministic substitutes for the upstream state-assembly dependencies:
        //  * secrets_configured stands in for fetch_raw_secrets / the secret-management client.
        //  * superposition_backend resolves the storage choice through the superposition client.
        //  * process_tracker_id records the async constructor as a scheduled tracker.
        const bool secrets_configured =
            m_context.secret_resolver().resolve("jwt", "jwt_secret").has_value();
        const auto resolved =
            m_context.superposition().resolve("app_state", "storage_impl", service_name);
        const auto superposition_backend = resolved ? resolved->get_value() : storage_impl;
        serde::Value::Object tracker_body{};
        tracker_body["session_id"] = serde::Value{session_id};
        tracker_body["storage_impl"] = serde::Value{storage_impl};
        const auto payload = m_support.serialize_document(serde::Value{std::move(tracker_body)});
        const auto process_tracker_id =
            m_context.process_trackers().schedule("app_state_with_storage", payload, "0");

        m_context.cache().put(
            "app.with_storage:" + session_id,
            core::cache::to_cache_value(storage_impl)
        );

        m_context.metrics().record_flow("AppStateWithStorage");
        m_context.metrics().counter("APP_STATE_WITH_STORAGE").increment();

        serde::Value::Object document{};
        document.insert(
            std::string{with_storage_wire_fields()[0]},
            serde::Value{std::string{"app"}}
        );
        document.insert(
            std::string{with_storage_wire_fields()[1]},
            serde::Value{std::string{"with_storage"}}
        );
        document.insert(std::string{with_storage_wire_fields()[2]}, serde::Value{session_id});
        document.insert(std::string{with_storage_wire_fields()[3]}, serde::Value{storage_impl});
        document.insert(std::string{with_storage_wire_fields()[4]}, serde::Value{service_name});
        document.insert(std::string{with_storage_wire_fields()[5]}, serde::Value{storage_ok});
        document.insert(
            std::string{with_storage_wire_fields()[6]},
            serde::Value{secrets_configured}
        );
        document.insert(
            std::string{with_storage_wire_fields()[7]},
            serde::Value{superposition_backend}
        );
        document.insert(
            std::string{with_storage_wire_fields()[8]},
            serde::Value{process_tracker_id}
        );
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    AppRuntimeSupport m_support;
};

} // namespace app::handlers::app
