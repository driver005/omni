export module app_user:user_role_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :user_role_input;
import :user_role_result;
import :user_role_runtime;
import :user_role_handlers_v1;
import :user_role_handlers_v2;

export namespace app::handlers::user_role {

// Facade ("handler of handlers"). The original 941-line handler is split across
// :handlers_v1 and :handlers_v2. Both children receive references to the ONE
// UserRoleRuntime and ONE UserRoleInputSupport owned here, so mutable state is
// never duplicated. install() replays the original 24 add_route registrations
// in their exact order (registry order is observable).
class UserRoleRouteHandlers
{
public:
    explicit UserRoleRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support, m_request_support},
        m_v2{context, m_runtime, m_input_support, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install_route_01(router);
        m_v1.install_route_02(router);
        m_v2.install_route_03(router);
        m_v2.install_route_04(router);
        m_v1.install_route_05(router);
        m_v2.install_route_06(router);
        m_v1.install_route_07(router);
        m_v2.install_route_08(router);
        m_v1.install_route_09(router);
        m_v1.install_route_10(router);
        m_v1.install_route_11(router);
        m_v2.install_route_12(router);
        m_v1.install_route_13(router);
        m_v2.install_route_14(router);
        m_v1.install_route_15(router);
        m_v2.install_route_16(router);
        m_v1.install_route_17(router);
        m_v1.install_route_18(router);
        m_v1.install_route_19(router);
        m_v1.install_route_20(router);
        m_v2.install_route_21(router);
        m_v1.install_route_22(router);
        m_v1.install_route_23(router);
        m_v1.install_route_24(router);
    }

private:
    UserRoleRuntime m_runtime;
    UserRoleInputSupport m_input_support;

    UserRolePolicy m_policy{m_input_support};

    UserRoleRequestSupport m_request_support{m_policy};
    UserRoleHandlersV1 m_v1;
    UserRoleHandlersV2 m_v2;
};

} // namespace app::handlers::user_role
