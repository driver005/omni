export module app_auth:authentication_runtime_eligibility_check;

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

class AuthenticationEligibilityCheckRuntime
{
public:
    explicit AuthenticationEligibilityCheckRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: eligibility-check ─────────────────────────
    // unified_authentication_service.rs:1685-1734 authentication_eligibility_check_core.
    [[nodiscard]] core::behavior::Outcome<serde::Value> eligibility_check(
        const ::app::dto::authentication::AuthenticationEligibilityCheckRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
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
        // :1722-1727 EligibilityHandler::run_check(StoreEligibilityCheckData):
        // the check stores the request's eligibility_check_data in Redis and yields
        // AuthSdkNextAction::AwaitMerchantCallback; otherwise Proceed.
        const bool should_store =
            m_support.eligibility_data_should_be_stored(record->get_merchant_id());
        if (should_store) {
            m_support.store_eligibility_data(
                record->get_merchant_id(),
                authentication_id,
                m_support.eligibility_data_from_request(
                    request.get_eligibility_check_data(),
                    detail::request_body(ctx)
                )
            );
        }
        m_support.update_eligibility_check_trackers(authentication_id, flow);
        m_context.logger().record_flow("AuthenticationEligibilityCheck");
        const char* action = should_store ? "await_merchant_callback" : "proceed";
        JsonObject sdk_next_action{};
        sdk_next_action.val(action, serde::Value{serde::Value::Object{}});
        return JsonObject{}
            .str("authentication_id", std::string{authentication_id})
            .val("sdk_next_action", sdk_next_action.done())
            .done();
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
