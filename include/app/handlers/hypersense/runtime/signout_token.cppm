export module app_handlers_hypersense:runtime_signout_token;

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
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::hypersense {

class HypersenseSignoutTokenRuntime
{
public:
    explicit HypersenseSignoutTokenRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/hypersense.rs:51 -> core/external_service_auth.rs:55-63. NoAuth.
    [[nodiscard]] std::expected<SignoutTokenResult, core::error::AppError>
    signout_token(const SignoutTokenInput& input) const
    {
        m_support.record_flow("HypersenseSignoutToken", LockActionNotApplicable{});
        m_context.metrics().counter("hypersense.signout_token").increment();

        // core/external_service_auth.rs:55-57 — decode_jwt::<ExternalToken> ->
        // Unauthorized.
        const auto claims = m_support.decode_jwt_external(input.request.get_token());
        if (!claims) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized")
            };
        }

        // core/external_service_auth.rs:59-61 — insert_user_in_blacklist(&state,
        // &token.user_id) -> InvalidJwtToken on failure. The durable config row plus
        // its cache mirror are the local Redis stand-in.
        if (!m_support.insert_user_in_blacklist(claims->user_id)) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::UNAUTHORIZED, "InvalidJwtToken")
            };
        }

        // core/external_service_auth.rs:63 — ApplicationResponse::StatusOk (bodyless).
        return SignoutTokenResult{};
    }

private:
    app::context::AppContext& m_context;
    HypersenseRuntimeSupport m_support;
};

} // namespace app::handlers::hypersense
