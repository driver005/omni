export module app_auth:hypersense_runtime_verify_token;

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
import app_services;

import :hypersense_input;
import :hypersense_result;
import :hypersense_runtime_common;

export namespace app::handlers::hypersense {

class HypersenseVerifyTokenRuntime
{
public:
    explicit HypersenseVerifyTokenRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/hypersense.rs:75 -> core/external_service_auth.rs:73-102.
    // NoAuth: the payload token is the only credential.
    [[nodiscard]] std::expected<HypersenseVerifyResponse, core::error::AppError>
    verify_token(const VerifyTokenInput& input) const
    {
        m_support.record_flow("HypersenseVerifyToken", LockActionNotApplicable{});
        m_context.metrics().counter("hypersense.verify_token").increment();

        // core/external_service_auth.rs:73-75 — decode_jwt::<ExternalToken> ->
        // Unauthorized.
        const auto claims = m_support.decode_jwt_external(input.request.get_token());
        if (!claims) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized")
            };
        }

        // core/external_service_auth.rs:77-81 — check_user_in_blacklist(user_id, exp)
        // -> InvalidJwtToken when true.
        if (m_support.check_user_in_blacklist(claims->user_id, claims->exp)) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::UNAUTHORIZED, "InvalidJwtToken")
            };
        }

        // core/external_service_auth.rs:83 — token.check_service_type(&required) ->
        // AccessForbidden { resource: "hypersense" }.
        if (!m_support.check_service_type(*claims, ExternalServiceType::Hypersense)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                std::format("AccessForbidden: {}", kHypersenseServiceType)
            )};
        }

        // core/external_service_auth.rs:85-90 — global_store.find_active_user_by_user_id
        // -> InternalServerError ("User not found in database").
        const auto user = m_support.find_active_user_by_user_id(claims->user_id);
        if (!user) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "User not found in database"
            )};
        }

        // core/external_service_auth.rs:95-102 — ExternalVerifyTokenResponse::Hypersense
        // { user_id (from DB), merchant_id (from token), name, email (from DB) }.
        HypersenseVerifyResponse response{};
        response.set_user_id(user->get_user_id());
        response.set_merchant_id(claims->merchant_id);
        response.set_name(user->get_name());
        response.set_email(user->get_email());
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    HypersenseRuntimeSupport m_support;
};

} // namespace app::handlers::hypersense
