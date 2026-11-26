export module app_auth:authentication_runtime_authenticate;

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

class AuthenticationAuthenticateRuntime
{
public:
    explicit AuthenticationAuthenticateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: authenticate ─────────────────────────
    // unified_authentication_service.rs:1354-1518 authentication_authenticate_core.
    [[nodiscard]] std::expected<AuthenticationAuthenticateResult, core::error::AppError>
    authenticate(const AuthenticationAuthenticateInput& input) const
    {
        auto record = m_support.find_authentication(input.authentication_id);
        if (!record.has_value()) {
            return std::unexpected{m_support.not_found_error(input.authentication_id)};
        }
        if (const auto err =
                m_support.authenticate_client_secret(*record, input.auth.client_secret)) {
            return std::unexpected{*err};
        }
        // :1440-1465 ExternalAuthentication::authentication (AReq) + vault CAVV lookup.
        const auto outcome =
            m_support.perform_authentication(*record, input.auth.flow, input.request);
        m_support.update_trackers_from_outcome(*record, outcome);
        m_support.save_authentication(*record);
        m_context.metrics().record_flow("AuthenticationAuthenticate");

        // :1485-1506 CAVV/ECI are returned only for AuthFlow::Merchant on a Success trans_status.
        const bool include_cavv =
            input.auth.flow == AuthFlowKind::Merchant && outcome.trans_status == "Y";
        return AuthenticationAuthenticateResult{
            m_support.build_authenticate_response(*record, include_cavv, outcome)
        };
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
