export module app_auth:authentication_runtime_eligibility;

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

class AuthenticationEligibilityRuntime
{
public:
    explicit AuthenticationEligibilityRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: eligibility ─────────────────────────
    // unified_authentication_service.rs:1032-1351 authentication_eligibility_core.
    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    eligibility(const AuthenticationEligibilityInput& input) const
    {
        // :1048-1059 find_authentication... -> AuthenticationNotFound.
        auto record = m_support.find_authentication(input.authentication_id);
        if (!record.has_value()) {
            return std::unexpected{m_support.not_found_error(input.authentication_id)};
        }
        // :1063-1071 authenticate_authentication_client_secret_and_check_expiry.
        if (const auto err =
                m_support.authenticate_client_secret(*record, input.auth.client_secret)) {
            return std::unexpected{*err};
        }
        // :1073 ensure_not_terminal_status(trans_status).
        if (const auto err = m_support.ensure_not_terminal_status(*record)) {
            return std::unexpected{*err};
        }
        // :1075-1084 profile_id must match the authentication's profile_id.
        if (input.request.get_profile_id().has_value() &&
            input.request.get_profile_id()->get_value() != record->get_profile_id()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "profile_id must match the authentication's profile_id"
            )};
        }
        // :1105-1120 notification_url: return_url or default redirect URL.
        const std::string notification_url =
            record->get_return_url().empty()
                ? std::string{"/authentication/"} + input.authentication_id + "/redirect"
                : record->get_return_url();

        // :1284-1306 ExternalAuthentication::pre_authentication via do_auth_connector_call.
        const auto outcome =
            m_support.perform_pre_authentication("eligibility", *record, input.auth.flow);
        // :1316-1331 external_authentication_update_trackers.
        m_support.update_trackers_from_outcome(*record, outcome);
        m_support.save_authentication(*record);
        m_context.metrics().record_flow("AuthenticationEligibility");

        // :1333-1346 foreign_try_from with req.get_next_action_api(base_url, auth_id).
        // :1342-1345 echoes the request's billing / shipping / browser / email back verbatim.
        return m_support.build_eligibility_response(
            *record,
            input.authentication_id,
            input.request,
            notification_url,
            input.body
        );
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
