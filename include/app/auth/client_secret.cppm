export module app_auth:client_secret_handler;

import std;
import core_router;
import core_behavior;
import app_context;

import :client_secret_input;
import :client_secret_result;
import :client_secret_runtime;
import :client_secret_handlers_v2;

export namespace app::handlers::client_secret {

class ClientSecretRouteHandlers
{
public:
    explicit ClientSecretRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_policy{context},
        m_request_support{m_policy},
        m_v2{context, m_runtime, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    ClientSecretRuntime m_runtime;
    ClientSecretPolicy m_policy;
    core::behavior::RequestSupport<ClientSecretPolicy> m_request_support;
    ClientSecretHandlersV2 m_v2;
};

} // namespace app::handlers::client_secret
