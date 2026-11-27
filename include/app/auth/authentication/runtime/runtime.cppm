export module app_auth:authentication_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_dto;

import :authentication_input;
import :authentication_result;
import :authentication_runtime_common;
import :authentication_runtime_create;
import :authentication_runtime_eligibility;
import :authentication_runtime_authenticate;
import :authentication_runtime_eligibility_check;
import :authentication_runtime_retrieve_eligibility_check;
import :authentication_runtime_sync;
import :authentication_runtime_sync_post_update;
import :authentication_runtime_session_token;

export namespace app::handlers::authentication {

class AuthenticationRuntime
{
public:
    explicit AuthenticationRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_eligibility{context},
        m_authenticate{context},
        m_eligibility_check{context},
        m_retrieve_eligibility_check{context},
        m_sync{context},
        m_sync_post_update{context},
        m_session_token{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::authentication::AuthenticationResponse>
    create(
        const ::app::dto::authentication::AuthenticationCreateRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> eligibility(
        const ::app::dto::authentication::AuthenticationEligibilityRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_eligibility.eligibility(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::authentication::AuthenticationAuthenticateResponse>
    authenticate(
        const ::app::dto::authentication::AuthenticationAuthenticateRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_authenticate.authenticate(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> eligibility_check(
        const ::app::dto::authentication::AuthenticationEligibilityCheckRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_eligibility_check.eligibility_check(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_eligibility_check(const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_eligibility_check.retrieve_eligibility_check(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::authentication::AuthenticationSyncResponse>
    sync(
        const ::app::dto::authentication::AuthenticationSyncRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_sync.sync(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    sync_post_update(const core::behavior::CallContext& ctx) const
    {
        return m_sync_post_update.sync_post_update(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> session_token(
        const ::app::dto::authentication::AuthenticationSessionTokenRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_session_token.session_token(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationCreateRuntime m_create;
    AuthenticationEligibilityRuntime m_eligibility;
    AuthenticationAuthenticateRuntime m_authenticate;
    AuthenticationEligibilityCheckRuntime m_eligibility_check;
    AuthenticationRetrieveEligibilityCheckRuntime m_retrieve_eligibility_check;
    AuthenticationSyncRuntime m_sync;
    AuthenticationSyncPostUpdateRuntime m_sync_post_update;
    AuthenticationSessionTokenRuntime m_session_token;
};

} // namespace app::handlers::authentication
