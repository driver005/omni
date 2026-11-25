export module app_handlers_authentication:runtime_session_token;

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
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::authentication {

class AuthenticationSessionTokenRuntime
{
public:
    explicit AuthenticationSessionTokenRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: session token ─────────────────────────
    // unified_authentication_service.rs:2516-2583 authentication_session_core.
    [[nodiscard]] std::expected<AuthenticationSessionTokenResult, core::error::AppError>
    session_token(const AuthenticationSessionTokenInput& input) const
    {
        auto record = m_support.find_authentication(input.authentication_id);
        if (!record.has_value()) {
            return std::unexpected{m_support.not_found_error(input.authentication_id)};
        }
        if (const auto err =
                m_support.authenticate_client_secret(*record, input.auth.client_secret)) {
            return std::unexpected{*err};
        }
        m_support.track_record_fetch("session_token", input.authentication_id);
        const auto tracker_id = m_support.schedule_auth_tracker(
            "session_token",
            input.authentication_id,
            input.auth.flow
        );
        m_context.process_trackers().mark_terminal(
            tracker_id,
            std::string{input.authentication_id}
        );
        m_context.metrics().record_flow("AuthenticationSessionToken");

        // :2562-2573 is_click_to_pay_enabled && authentication_product_ids ->
        // get_session_token_for_click_to_pay (:2586-2680), else session_tokens stays empty.
        bool click_to_pay_enabled = record->get_click_to_pay_enabled() == "true";
        if (click_to_pay_enabled && record->get_click_to_pay_product_ids().empty()) {
            click_to_pay_enabled = false;
        }
        serde::Value::Array tokens{};
        if (click_to_pay_enabled) {
            tokens.push_back(m_support.get_session_token_for_click_to_pay(*record));
        }
        return AuthenticationSessionTokenResult{
            JsonObject{}
                .str("authentication_id", std::string{input.authentication_id})
                .val("session_token", serde::Value{std::move(tokens)})
                .done()
        };
    }

private:
    app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
