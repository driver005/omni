export module app_handlers_recovery_webhooks:handler;

import std;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v2;

export namespace app::handlers::recovery_webhooks {

class RecoveryWebhooksRouteHandlers
{
public:
    explicit RecoveryWebhooksRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_v2{context, m_runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    RecoveryWebhooksRuntime m_runtime;
    RecoveryWebhooksHandlersV2 m_v2;
};

} // namespace app::handlers::recovery_webhooks
