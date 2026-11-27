export module app_auth:authentication_runtime_sync;

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

import :authentication_input;
import :authentication_result;
import :authentication_runtime_common;

export namespace app::handlers::authentication {

class AuthenticationSyncRuntime
{
public:
    explicit AuthenticationSyncRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: sync ─────────────────────────
    // unified_authentication_service.rs:2070-2383 authentication_sync_core.
    [[nodiscard]] std::
        expected<::app::dto::authentication::AuthenticationSyncResponse, core::error::AppError>
        sync(const AuthenticationSyncInput& input) const
    {
        auto record = m_support.find_authentication(input.authentication_id);
        if (!record.has_value()) {
            return std::unexpected{m_support.not_found_error(input.authentication_id)};
        }
        if (const auto err =
                m_support.authenticate_client_secret(*record, input.auth.client_secret)) {
            return std::unexpected{*err};
        }
        // :2232-2261 determine_auth_sync_strategy + execute_post_authentication_flow /
        // use_stored_authentication_value / NoOperation.
        const auto strategy = m_support.determine_auth_sync_strategy(*record, input.auth.flow);
        ConnectorOutcome outcome{};
        if (strategy == AuthSyncStrategy::ExecutePostAuth) {
            outcome = m_support.perform_post_authentication(*record, input.auth.flow);
            m_support.update_trackers_from_outcome(*record, outcome);
        } else if (strategy == AuthSyncStrategy::UseStoredAuthValue) {
            record->set_cavv(m_support.cavv_from(record->get_threeds_server_transaction_id()));
        }
        m_support.save_authentication(*record);
        m_context.metrics().record_flow("AuthenticationSync");

        // :2331-2379 AuthenticationSyncResponse built entirely from the updated record.
        return m_support.build_sync_response(*record, input.merchant_id, input.auth.flow, outcome);
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
