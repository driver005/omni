export module app_payment:payments_handler;

import std;
import interfaces;
import core_router;
import app_context;
import :payments_input;
import :payments_result;
import :payments_runtime;
import :payments_handlers_v1;
import :payments_handlers_v2;

export namespace app::handlers::payments {

// Facade: one shared scope / input support / runtime, child handlers hold references.
class PaymentsRouteHandlers
{
public:
    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
        m_v1.install(router);
    }

    explicit PaymentsRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_scope{},
        m_input_support{context, m_scope},
        m_runtime{context, m_scope},
        m_v2{context, m_scope, m_input_support, m_runtime},
        m_v1{context, m_scope, m_input_support, m_runtime},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_v2.bind_request_support(m_request_support);
        m_v1.bind_request_support(m_request_support);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState m_scope{};
    PaymentsInputSupport m_input_support;
    PaymentsRuntime m_runtime;
    PaymentsHandlersV2 m_v2;
    PaymentsHandlersV1 m_v1;
    PaymentsPolicy m_policy;
    PaymentsRequestSupport m_request_support;
};

} // namespace app::handlers::payments
