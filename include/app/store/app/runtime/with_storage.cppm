export module app_store:app_runtime_with_storage;

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

class WithStorageRuntime
{
public:
    explicit WithStorageRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream AppState::with_storage() is an async constructor (app.rs:497-643), no auth.
    [[nodiscard]] std::expected<WithStorageResult, core::error::AppError>
    with_storage(const WithStorageInput& input) const
    {
        const auto storage_impl = input.storage_impl;
        const auto service_name = input.service_name;
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

        WithStorageResult result{};
        result.set_partition("app");
        result.set_op("with_storage");
        result.set_session_id(session_id);
        result.set_storage_impl(storage_impl);
        result.set_service_name(service_name);
        result.set_storage_ok(storage_ok);
        result.set_secrets_configured(secrets_configured);
        result.set_superposition_backend(superposition_backend);
        result.set_process_tracker_id(process_tracker_id);
        return result;
    }

private:
    ::app::context::AppContext& m_context;
    AppRuntimeSupport m_support;
};

} // namespace app::handlers::app
