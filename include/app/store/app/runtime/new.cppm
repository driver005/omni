export module app_store:app_runtime_new;

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

class NewRuntime
{
public:
    explicit NewRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream AppState::new() delegates to with_storage(Postgresql) (app.rs:729-743).
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    app_new(const core::behavior::CallContext& ctx) const
    {
        const auto service_name = m_support.query_or(ctx, "service_name", "default");
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

        serde::Value::Object document{};
        document.insert(std::string{new_wire_fields()[0]}, serde::Value{std::string{"app"}});
        document.insert(std::string{new_wire_fields()[1]}, serde::Value{std::string{"new"}});
        document.insert(std::string{new_wire_fields()[2]}, serde::Value{session_id});
        document.insert(std::string{new_wire_fields()[3]}, serde::Value{storage_impl});
        document.insert(std::string{new_wire_fields()[4]}, serde::Value{service_name});
        document.insert(std::string{new_wire_fields()[5]}, serde::Value{storage_ok});
        document.insert(std::string{new_wire_fields()[6]}, serde::Value{process_tracker_id});
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    AppRuntimeSupport m_support;
};

} // namespace app::handlers::app
