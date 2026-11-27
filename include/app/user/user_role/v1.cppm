export module app_user:user_role_handlers_v1;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :user_role_input;
import :user_role_result;
import :user_role_runtime;
import :user_role_handlers_v1_roles_crud;
import :user_role_handlers_v1_invites;
import :user_role_handlers_v1_list_info;

export namespace app::handlers::user_role {

// V1 version facade. Replays the original add_route order for unversioned
// routes; the root facade interleaves these calls with the v2 routes.
class UserRoleHandlersV1
{
public:
    UserRoleHandlersV1(
        ::app::context::AppContext& context,
        UserRoleRuntime& runtime,
        UserRoleInputSupport& input_support,
        UserRoleRequestSupport& request_support
    ) noexcept :
        m_roles_crud{context, runtime, input_support, request_support},
        m_invites{context, runtime, input_support, request_support},
        m_list_info{context, runtime, input_support, request_support}
    {
    }

    void install_route_01(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_01(router);
    }

    void install_route_02(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_02(router);
    }

    void install_route_05(core::router::RouterContext<>& router)
    {
        m_roles_crud.install_route_05(router);
    }

    void install_route_07(core::router::RouterContext<>& router)
    {
        m_roles_crud.install_route_07(router);
    }

    void install_route_09(core::router::RouterContext<>& router)
    {
        m_roles_crud.install_route_09(router);
    }

    void install_route_10(core::router::RouterContext<>& router)
    {
        m_roles_crud.install_route_10(router);
    }

    void install_route_11(core::router::RouterContext<>& router)
    {
        m_invites.install_route_11(router);
    }

    void install_route_13(core::router::RouterContext<>& router)
    {
        m_invites.install_route_13(router);
    }

    void install_route_15(core::router::RouterContext<>& router)
    {
        m_roles_crud.install_route_15(router);
    }

    void install_route_17(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_17(router);
    }

    void install_route_18(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_18(router);
    }

    void install_route_19(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_19(router);
    }

    void install_route_20(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_20(router);
    }

    void install_route_22(core::router::RouterContext<>& router)
    {
        m_invites.install_route_22(router);
    }

    void install_route_23(core::router::RouterContext<>& router)
    {
        m_list_info.install_route_23(router);
    }

    void install_route_24(core::router::RouterContext<>& router)
    {
        m_invites.install_route_24(router);
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_01(router);
        install_route_02(router);
        install_route_05(router);
        install_route_07(router);
        install_route_09(router);
        install_route_10(router);
        install_route_11(router);
        install_route_13(router);
        install_route_15(router);
        install_route_17(router);
        install_route_18(router);
        install_route_19(router);
        install_route_20(router);
        install_route_22(router);
        install_route_23(router);
        install_route_24(router);
    }

private:
    UserRoleV1RolesCrudHandlers m_roles_crud;
    UserRoleV1InvitesHandlers m_invites;
    UserRoleV1ListInfoHandlers m_list_info;
};

} // namespace app::handlers::user_role
