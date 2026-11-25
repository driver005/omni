export module app_handlers_client_secret:handler;

import std;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v2;

export namespace app::handlers::client_secret {

class ClientSecretRouteHandlers
{
public:
    explicit ClientSecretRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_v2{context, m_runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    ClientSecretRuntime m_runtime;
    ClientSecretHandlersV2 m_v2;
};

} // namespace app::handlers::client_secret
