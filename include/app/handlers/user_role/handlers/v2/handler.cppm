export module app_handlers_user_role:handlers_v2;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v2_roles_crud;
import :handlers_v2_list_info;
import :handlers_v2_aliases;

export namespace app::handlers::user_role {

// V2 version facade. Owns distinct v2 methods and the v2 alias adapters.
// The root facade interleaves these calls with the v1 routes at original positions.
class UserRoleHandlersV2
{
public:
    UserRoleHandlersV2(
        app::context::AppContext& context,
        UserRoleRuntime& runtime,
        UserRoleInputSupport& input_support
    ) noexcept :
        m_roles_crud{context, runtime, input_support},
        m_list_info{context, runtime, input_support},
        m_aliases{context, runtime, input_support}
    {
    }

    void install_route_03(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_03(router);
    }

    void install_route_04(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_04(router);
    }

    void install_route_06(core::router::RouterContext<>& router)
    {
        m_roles_crud.install_route_06(router);
    }

    void install_route_08(core::router::RouterContext<>& router)
    {
        m_roles_crud.install_route_08(router);
    }

    void install_route_12(core::router::RouterContext<>& router)
    {
        m_aliases.install_route_12(router);
    }

    void install_route_14(core::router::RouterContext<>& router)
    {
        m_aliases.install_route_14(router);
    }

    void install_route_16(core::router::RouterContext<>& router)
    {
        m_aliases.install_route_16(router);
    }

    void install_route_21(core::router::RouterContext<>& router)
    {
        m_aliases.install_route_21(router);
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_03(router);
        install_route_04(router);
        install_route_06(router);
        install_route_08(router);
        install_route_12(router);
        install_route_14(router);
        install_route_16(router);
        install_route_21(router);
    }

private:
    UserRoleV2RolesCrudHandlers m_roles_crud;
    UserRoleV2ListInfoHandlers m_list_info;
    UserRoleV2AliasesHandlers m_aliases;
};

} // namespace app::handlers::user_role
