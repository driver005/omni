export module app_user:user_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :user_runtime;
import :user_input;
import :user_result;
import :user_handlers_v1;
import :user_handlers_v2;

export namespace app::handlers::user {

class UserRouteHandlers
{
public:
    explicit UserRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support, m_request_support},
        m_v2{context, m_runtime, m_input_support, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
        m_v2.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntime m_runtime;
    UserInputSupport m_input_support;

    UserPolicy m_policy{m_input_support};

    UserRequestSupport m_request_support{m_policy};
    UserHandlersV1 m_v1;
    UserHandlersV2 m_v2;
};

} // namespace app::handlers::user
