export module app_auth:external_service_auth_runtime_validate_token;

import std;
import interfaces;
import core_serde;
import app_dto;
import app_context;
import core_error;
import app_crypto;
import app_metrics;

import :external_service_auth_input;
import :external_service_auth_result;
import :external_service_auth_runtime_common;

export namespace app::handlers::external_service_auth {

class ValidateTokenRuntime
{
public:
    explicit ValidateTokenRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/external_service_auth.rs:11-27 — validate_token.
    // core/external_service_auth.rs:111-196 validate_token_core.
    [[nodiscard]] std::expected<ValidateTokenResult, core::error::AppError>
    validate_token(const ValidateTokenInput& input) const
    {
        // core/external_service_auth.rs:111 — req.token.expose().
        const ::app::crypto::SecretString token_secret{input.request.get_token()};
        const auto token = token_secret.value();

        // core/external_service_auth.rs:113 — decode_jwt::<AuthToken>(&token, &state).
        const auto decoded = m_support.decode_jwt(token);
        if (decoded.outcome == JwtDecodeOutcome::Expired) {
            return std::unexpected{m_support.expired_jwt_error()};
        }
        if (decoded.outcome != JwtDecodeOutcome::Valid) {
            return std::unexpected{m_support.invalid_jwt_error()};
        }
        const auto& payload = decoded.token;

        // core/external_service_auth.rs:115-117 — payload.check_in_blacklist(&state).
        const auto blacklist = m_support.check_in_blacklist(payload);
        if (blacklist == BlacklistOutcome::Blacklisted) {
            return std::unexpected{m_support.invalid_jwt_error()};
        }
        if (blacklist == BlacklistOutcome::IdleExpired) {
            return std::unexpected{m_support.expired_jwt_error()};
        }

        // core/external_service_auth.rs:119 — check_tenant(payload.tenant_id, state.tenant).
        if (!m_support.check_tenant(payload.tenant_id)) {
            return std::unexpected{m_support.invalid_jwt_error()};
        }

        // core/external_service_auth.rs:121 — get_role_info(&state, &payload).
        const auto role_info = m_support.get_role_info(payload);
        if (!role_info) {
            return std::unexpected{m_support.invalid_jwt_error()};
        }

        // core/external_service_auth.rs:123-136 — match ValidatingService::OfferEngine.
        switch (input.request.get_service()) {
            case ::app::dto::external_service_auth::ValidatingService::OfferEngine:
                {
                    // core:125 — resolve_offer_engine_merchant_id.
                    const auto merchant_id = m_support.resolve_offer_engine_merchant_id(payload);
                    if (!merchant_id) {
                        return std::unexpected{m_support.access_forbidden_error()};
                    }

                    // routes/external_service_auth.rs:16-26 — Flow::ExternalServiceValidateToken,
                    // LockAction::NotApplicable (no lock acquired). Local metrics substitute.
                    m_context.metrics().record_flow("ExternalServiceValidateToken");
                    m_context.metrics().counter("EXTERNAL_SERVICE_TOKEN_VALIDATED").increment();

                    // Typed OfferEngine arm of the untagged upstream response enum; the
                    // DTO's Serializable emits the exact field order from api_models:40-45.
                    ::app::dto::external_service_auth::ExternalVerifyTokenResponse response{};
                    response.set_merchant_id(*merchant_id);
                    response.set_context(std::string{kOfferEngineContext});
                    response.set_token(
                        token_secret.masked_as(::app::crypto::SecretFieldKind::ClientSecret)
                    );
                    response.set_permissions(m_support.offer_engine_permissions(*role_info));
                    return ValidateTokenResult{std::move(response)};
                }
        }

        return std::unexpected{core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            "unsupported validating service"
        )};
    }

private:
    ::app::context::AppContext& m_context;
    ExternalServiceAuthRuntimeSupport m_support;
};

} // namespace app::handlers::external_service_auth
