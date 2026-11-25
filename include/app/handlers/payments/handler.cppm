export module app_handlers_payments:handler;

import std;
import interfaces;
import core_router;
import app_context;
import :input;
import :result;
import :runtime;
import :handlers_v1;
import :handlers_v2;

export namespace app::handlers::payments {

// Facade: one shared scope / input support / runtime, child handlers hold references.
class PaymentsRouteHandlers
{
public:
    void install(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
        m_v1.install(router);
    }

    explicit PaymentsRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{context},
        m_scope{},
        m_input_support{context, m_scope},
        m_runtime{context, m_scope},
        m_v2{context, m_scope, m_input_support, m_runtime},
        m_v1{context, m_scope, m_input_support, m_runtime}
    {
    }

private:
    app::context::AppContext& m_context;
    PaymentsScopeState m_scope{};
    PaymentsInputSupport m_input_support;
    PaymentsRuntime m_runtime;
    PaymentsHandlersV2 m_v2;
    PaymentsHandlersV1 m_v1;
};

} // namespace app::handlers::payments
