export module app_store:app_runtime_new;

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

class NewRuntime
{
public:
    explicit NewRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream AppState::new() delegates to with_storage(Postgresql) (app.rs:729-743).
    [[nodiscard]] std::expected<NewResult, core::error::AppError>
    app_new(const NewInput& input) const
    {
        const auto service_name = input.service_name;
        const auto session_id = m_context.ids().uuid_v4();
        const auto storage_impl = m_context.config_store()
                                      .get_value("app_state", "default_storage_impl")
                                      .value_or(std::string{"postgresql"});
        serde::Value::Object tracker_body{};
        tracker_body["session_id"] = serde::Value{session_id};
        const auto payload = m_support.serialize_document(serde::Value{std::move(tracker_body)});
        const auto process_tracker_id =
            m_context.process_trackers().schedule("app_state_new", payload, "0");
        m_context.cache().put("app.new:" + session_id, core::cache::to_cache_value(storage_impl));

        const bool storage_ok = m_support.storage_available();
        m_context.metrics().record_flow("AppStateNew");
        m_context.metrics().counter("APP_STATE_NEW").increment();

        NewResult result{};
        result.set_partition("app");
        result.set_op("new");
        result.set_session_id(session_id);
        result.set_storage_impl(storage_impl);
        result.set_service_name(service_name);
        result.set_storage_ok(storage_ok);
        result.set_process_tracker_id(process_tracker_id);
        return result;
    }

private:
    ::app::context::AppContext& m_context;
    AppRuntimeSupport m_support;
};

} // namespace app::handlers::app
