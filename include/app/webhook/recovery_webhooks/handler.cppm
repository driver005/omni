export module app_webhook:recovery_webhooks_handler;

import std;
import core_router;
import core_behavior;
import app_context;

import :recovery_webhooks_input;
import :webhook_policy;
import :recovery_webhooks_result;
import :recovery_webhooks_runtime;
import :recovery_webhooks_handlers_v2;

export namespace app::handlers::recovery_webhooks {

class RecoveryWebhooksRouteHandlers
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::RecoveryWebhooksPolicy>;

    explicit RecoveryWebhooksRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy},
        m_v2{context, m_runtime, m_input_support, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    RecoveryWebhooksRuntime m_runtime;
    RecoveryWebhooksInputSupport m_input_support;
    ::app::webhook::policy::RecoveryWebhooksPolicy m_policy;
    RequestSupport m_request_support;
    RecoveryWebhooksHandlersV2 m_v2;
};

} // namespace app::handlers::recovery_webhooks
