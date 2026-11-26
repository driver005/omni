export module app_webhook:recovery_webhooks_handler;

import std;
import core_router;
import app_context;

import :recovery_webhooks_input;
import :recovery_webhooks_result;
import :recovery_webhooks_runtime;
import :recovery_webhooks_handlers_v2;

export namespace app::handlers::recovery_webhooks {

class RecoveryWebhooksRouteHandlers
{
public:
    explicit RecoveryWebhooksRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_v2{context, m_runtime}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    RecoveryWebhooksRuntime m_runtime;
    RecoveryWebhooksHandlersV2 m_v2;
};

} // namespace app::handlers::recovery_webhooks
