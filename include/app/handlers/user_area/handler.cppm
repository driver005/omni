export module app_user:user_area_handler;

import std;
import core_router;
import app_context;

import :admin_base;
import :api_keys_base;
import :customers_base;
import :hierarchical_resources_base;
import :profile_acquirer_base;
import :profiles_base;
import :user_base;
import :user_role_base;

export namespace app::api {

class UserHandlers
{
public:
    explicit UserHandlers(::app::context::AppContext& ctx) noexcept :
        m_context{ctx},
        m_admin{ctx},
        m_api_keys{ctx},
        m_customers{ctx},
        m_hierarchical_resources{ctx},
        m_profile_acquirer{ctx},
        m_profiles{ctx},
        m_user{ctx},
        m_user_role{ctx}
    {
    }

    // Replays the original AppRoutes order for the user-area domains.
    void install(core::router::RouterContext<>& router)
    {
        m_admin.install_routes(router);
        m_api_keys.install_routes(router);
        m_customers.install_routes(router);
        m_hierarchical_resources.install_routes(router);
        m_profile_acquirer.install_routes(router);
        m_profiles.install_routes(router);
        m_user.install_routes(router);
        m_user_role.install_routes(router);
    }

private:
    ::app::context::AppContext& m_context;

    ::app::handlers::admin::AdminRouteHandlers m_admin;
    ::app::handlers::api_keys::ApiKeysRouteHandlers m_api_keys;
    ::app::handlers::customers::CustomersRouteHandlers m_customers;
    ::app::handlers::hierarchical_resources::HierarchicalResourcesRouteHandlers
        m_hierarchical_resources;
    ::app::handlers::profile_acquirer::ProfileAcquirerRouteHandlers m_profile_acquirer;
    ::app::handlers::profiles::ProfilesRouteHandlers m_profiles;
    ::app::handlers::user::UserRouteHandlers m_user;
    ::app::handlers::user_role::UserRoleRouteHandlers m_user_role;
};

} // namespace app::api
