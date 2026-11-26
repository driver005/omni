export module app_payment:refunds_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :refunds_input;
import :refunds_result;
import :refunds_runtime;
import :refunds_handlers_v1;
import :refunds_handlers_v2;

export namespace app::handlers::refunds {

// Facade: one shared runtime and input support; child handlers hold references.
class RefundsRouteHandlers
{
public:
    explicit RefundsRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
        m_v2.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime m_runtime;
    RefundsInputSupport m_input_support;
    RefundsHandlersV1 m_v1;
    RefundsHandlersV2 m_v2;
};

} // namespace app::handlers::refunds
