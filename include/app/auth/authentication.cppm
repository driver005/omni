export module app_auth:authentication_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :authentication_input;
import :authentication_result;
import :authentication_runtime;
import :authentication_handlers_create;
import :authentication_handlers_eligibility_authenticate;
import :authentication_handlers_eligibility_check_retrieve;
import :authentication_handlers_sync_redirect;
import :authentication_handlers_session_token;

export namespace app::handlers::authentication {

class AuthenticationRouteHandlers
{
public:
    explicit AuthenticationRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_create{context, m_runtime},
        m_eligibility_authenticate{context, m_runtime},
        m_eligibility_check_retrieve{context, m_runtime},
        m_sync_redirect{context, m_runtime},
        m_session_token{context, m_runtime}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_create.install(router);
        m_eligibility_authenticate.install(router);
        m_eligibility_check_retrieve.install(router);
        m_sync_redirect.install(router);
        m_session_token.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntime m_runtime; // SINGLE shared runtime
    CreateHandlers m_create;
    EligibilityAuthenticateHandlers m_eligibility_authenticate;
    EligibilityCheckRetrieveHandlers m_eligibility_check_retrieve;
    SyncRedirectHandlers m_sync_redirect;
    SessionTokenHandlers m_session_token;
};

} // namespace app::handlers::authentication
