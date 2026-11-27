export module app_auth:authentication_runtime_retrieve_eligibility_check;

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

class AuthenticationRetrieveEligibilityCheckRuntime
{
public:
    explicit AuthenticationRetrieveEligibilityCheckRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: retrieve eligibility-check ─────────────────────────
    // unified_authentication_service.rs:1737-1790 authentication_retrieve_eligibility_check_core.
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_eligibility_check(const core::behavior::CallContext& ctx) const
    {
        const auto& authentication_id = detail::path_segment(ctx, 1);
        const auto flow =
            detail::is_client_flow(ctx) ? AuthFlowKind::Client : AuthFlowKind::Merchant;
        auto record = m_support.find_authentication(authentication_id);
        if (!record.has_value()) {
            return std::unexpected{m_support.not_found_error(authentication_id)};
        }
        // :1742-1769 Redis get_key(AUTHENTICATION_ELIGIBILITY_CHECK_DATA_KEY_<merchant>_<auth_id>)
        // -> GenericNotFoundError("Eligibility check data not found ...").
        const auto data =
            m_support.load_eligibility_data(record->get_merchant_id(), authentication_id);
        if (!data.has_value()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Eligibility check data not found for authentication id: " +
                    std::string{authentication_id}
            )};
        }
        m_support.track_record_fetch("retrieve_eligibility_check", authentication_id);
        const auto tracker_id =
            m_support.schedule_auth_tracker("retrieve_eligibility_check", authentication_id, flow);
        m_context.process_trackers().mark_terminal(tracker_id, std::string{authentication_id});
        m_context.logger().record_flow("AuthenticationRetrieveEligibilityCheck");

        // :1770-1788 ClickToPayEligibilityCheckResponseData { visa, mastercard consumer_present }.
        const std::string visa = data->first.empty() ? "null" : data->first;
        const std::string mastercard = data->second.empty() ? "null" : data->second;
        const auto click_to_pay =
            JsonObject{}.raw("visa", visa).raw("mastercard", mastercard).done();
        return JsonObject{}
            .str("authentication_id", std::string{authentication_id})
            .val(
                "eligibility_check_data",
                JsonObject{}.val("click_to_pay_enrollment_status", click_to_pay).done()
            )
            .done();
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
