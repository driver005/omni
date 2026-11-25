export module app_handlers_api_keys:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v1;
import :handlers_v2;

export namespace app::handlers::api_keys {

class ApiKeysRouteHandlers
{
public:
    explicit ApiKeysRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v1.install(router);
        m_v2.install(router);
    }

private:
    app::context::AppContext& m_context;
    ApiKeysRuntime m_runtime;            // SINGLE runtime instance — shared by every family.
    ApiKeysInputSupport m_input_support; // SINGLE input helper — shared by every family.
    ApiKeysHandlersV1 m_v1;
    ApiKeysHandlersV2 m_v2;
};

} // namespace app::handlers::api_keys
