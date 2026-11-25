export module app_handlers_authentication:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_create;
import :runtime_eligibility;
import :runtime_authenticate;
import :runtime_eligibility_check;
import :runtime_retrieve_eligibility_check;
import :runtime_sync;
import :runtime_sync_post_update;
import :runtime_session_token;

export namespace app::handlers::authentication {

class AuthenticationRuntime
{
public:
    explicit AuthenticationRuntime(app::context::AppContext& context) noexcept :
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

    [[nodiscard]] std::expected<AuthenticationCreateResult, core::error::AppError>
    create(const AuthenticationCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<AuthenticationEligibilityResult, core::error::AppError>
    eligibility(const AuthenticationEligibilityInput& input) const
    {
        return m_eligibility.eligibility(input);
    }

    [[nodiscard]] std::expected<AuthenticationAuthenticateResult, core::error::AppError>
    authenticate(const AuthenticationAuthenticateInput& input) const
    {
        return m_authenticate.authenticate(input);
    }

    [[nodiscard]] std::expected<AuthenticationEligibilityCheckResult, core::error::AppError>
    eligibility_check(const AuthenticationEligibilityCheckInput& input) const
    {
        return m_eligibility_check.eligibility_check(input);
    }

    [[nodiscard]] std::expected<AuthenticationRetrieveEligibilityCheckResult, core::error::AppError>
    retrieve_eligibility_check(const AuthenticationRetrieveEligibilityCheckInput& input) const
    {
        return m_retrieve_eligibility_check.retrieve_eligibility_check(input);
    }

    [[nodiscard]] std::expected<AuthenticationSyncResult, core::error::AppError>
    sync(const AuthenticationSyncInput& input) const
    {
        return m_sync.sync(input);
    }

    [[nodiscard]] std::expected<AuthenticationSyncPostUpdateResult, core::error::AppError>
    sync_post_update(const AuthenticationSyncPostUpdateInput& input) const
    {
        return m_sync_post_update.sync_post_update(input);
    }

    [[nodiscard]] std::expected<AuthenticationSessionTokenResult, core::error::AppError>
    session_token(const AuthenticationSessionTokenInput& input) const
    {
        return m_session_token.session_token(input);
    }

private:
    app::context::AppContext& m_context;
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
