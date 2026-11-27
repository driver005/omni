export module app_auth:authentication_runtime_session_token;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :call_context;
import :authentication_input;
import :authentication_result;
import :authentication_runtime_common;

export namespace app::handlers::authentication {

class AuthenticationSessionTokenRuntime
{
public:
    explicit AuthenticationSessionTokenRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: session token ─────────────────────────
    // unified_authentication_service.rs:2516-2583 authentication_session_core.
    [[nodiscard]] core::behavior::Outcome<serde::Value> session_token(
        const ::app::dto::authentication::AuthenticationSessionTokenRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        (void)request;
        const auto& authentication_id = detail::path_segment(ctx, 1);
        const auto flow =
            detail::is_client_flow(ctx) ? AuthFlowKind::Client : AuthFlowKind::Merchant;
        auto record = m_support.find_authentication(authentication_id);
        if (!record.has_value()) {
            return std::unexpected{m_support.not_found_error(authentication_id)};
        }
        if (const auto err = m_support.authenticate_client_secret(*record, ctx.client_secret)) {
            return std::unexpected{*err};
        }
        m_support.track_record_fetch("session_token", authentication_id);
        const auto tracker_id =
            m_support.schedule_auth_tracker("session_token", authentication_id, flow);
        m_context.process_trackers().mark_terminal(tracker_id, std::string{authentication_id});
        m_context.logger().record_flow("AuthenticationSessionToken");

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
        return JsonObject{}
            .str("authentication_id", std::string{authentication_id})
            .val("session_token", serde::Value{std::move(tokens)})
            .done();
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
