export module app_webhook:webhook_router;

import std;
import core_router;
import app_context;

import :webhooks_router;
import :webhook_events_router;
import :recovery_webhooks_router;
import :subscription_router;
import :verification_router;

export namespace app::api {

class WebhookHandlers
{
public:
    explicit WebhookHandlers(::app::context::AppContext& ctx) noexcept :
        m_context{ctx},
        m_recovery_webhooks{ctx},
        m_subscription{ctx},
        m_verification{ctx},
        m_webhook_events{ctx},
        m_webhooks{ctx}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        // Original global AppRoutes order for this area.
        m_recovery_webhooks.install_routes(router);
        m_subscription.install_routes(router);
        m_verification.install_routes(router);
        m_webhook_events.install_routes(router);
        m_webhooks.install_routes(router);
    }

private:
    ::app::context::AppContext& m_context;
    ::app::handlers::recovery_webhooks::RecoveryWebhooksRouteHandlers m_recovery_webhooks;
    ::app::handlers::subscription::SubscriptionRouteHandlers m_subscription;
    ::app::handlers::verification::VerificationRouteHandlers m_verification;
    ::app::handlers::webhook_events::WebhookEventsRouteHandlers m_webhook_events;
    ::app::handlers::webhooks::WebhooksRouteHandlers m_webhooks;
};

} // namespace app::api
