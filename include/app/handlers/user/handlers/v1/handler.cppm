export module app_handlers_user:handlers_v1;

import std;
import interfaces;
import core_router;
import app_context;
import :runtime;
import :input;
import :result;
import :handlers_v1_dashboard;
import :handlers_v1_auth;
import :handlers_v1_users;
import :handlers_v1_org;
import :handlers_v1_password;
import :handlers_v1_invites;
import :handlers_v1_two_factor;
import :handlers_v1_auth_methods;
import :handlers_v1_connector;
import :handlers_v1_internal;

export namespace app::handlers::user {

class UserHandlersV1
{
public:
    UserHandlersV1(
        app::context::AppContext& ctx,
        UserRuntime& runtime,
        UserInputSupport& input
    ) noexcept :
        m_dashboard{ctx, runtime, input},
        m_auth{ctx, runtime, input},
        m_users{ctx, runtime, input},
        m_org{ctx, runtime, input},
        m_password{ctx, runtime, input},
        m_invites{ctx, runtime, input},
        m_two_factor{ctx, runtime, input},
        m_auth_methods{ctx, runtime, input},
        m_connector{ctx, runtime, input},
        m_internal{ctx, runtime, input}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_dashboard.install(router);
        m_auth.install(router);
        m_users.install(router);
        m_org.install(router);
        m_password.install(router);
        m_invites.install(router);
        m_two_factor.install(router);
        m_auth_methods.install(router);
        m_connector.install(router);
        m_internal.install(router);
    }

private:
    UserHandlersV1Dashboard m_dashboard;
    UserHandlersV1Auth m_auth;
    UserHandlersV1Users m_users;
    UserHandlersV1Org m_org;
    UserHandlersV1Password m_password;
    UserHandlersV1Invites m_invites;
    UserHandlersV1TwoFactor m_two_factor;
    UserHandlersV1AuthMethods m_auth_methods;
    UserHandlersV1Connector m_connector;
    UserHandlersV1Internal m_internal;
};

} // namespace app::handlers::user
