export module app_auth:hypersense_runtime_signout_token;

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
import app_services;

import :hypersense_input;
import :hypersense_result;
import :hypersense_runtime_common;

export namespace app::handlers::hypersense {

class HypersenseSignoutTokenRuntime
{
public:
    explicit HypersenseSignoutTokenRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/hypersense.rs:51 -> core/external_service_auth.rs:55-63. NoAuth.
    [[nodiscard]] core::behavior::Outcome<void> signout_token(
        const ::app::dto::external_service_auth::ExternalSignoutTokenRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        (void)ctx;
        m_support.record_flow("HypersenseSignoutToken", LockActionNotApplicable{});
        m_context.logger().counter("hypersense.signout_token").increment();

        // core/external_service_auth.rs:55-57 — decode_jwt::<ExternalToken> ->
        // Unauthorized.
        const auto claims = m_support.decode_jwt_external(request.get_token());
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
        return {};
    }

private:
    ::app::context::AppContext& m_context;
    HypersenseRuntimeSupport m_support;
};

} // namespace app::handlers::hypersense
