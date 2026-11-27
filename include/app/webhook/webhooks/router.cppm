export module app_webhook:webhooks_router;

import std;
import interfaces;
import core_router;
import core_behavior;
import app_context;

import :webhooks_input;
import :webhook_policy;
import :webhooks_result;
import :webhooks_runtime;
import :webhooks_handlers_common;
import :webhooks_handlers_v1;
import :webhooks_handlers_v2;

export namespace app::handlers::webhooks {

/** @brief Facade ("handler of handlers").
 *
 *  Owns the single shared WebhooksRuntime and WebhooksInputSupport; version
 *  partitions only hold references to them. No business logic lives here.
 */
class WebhooksRouteHandlers
{
public:
    using RequestSupport = ::core::behavior::RequestSupport<::app::webhook::policy::WebhooksPolicy>;

    explicit WebhooksRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy},
        m_v1{context, m_runtime, m_input_support, m_request_support},
        m_v2{context, m_runtime, m_input_support, m_request_support},
        m_common{context, m_runtime, m_input_support, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        // Original registration order: v1 (3 routes), v2 (3 routes), then the
        // unversioned network-token routes (3 routes).
        m_v1.install(router);
        m_v2.install(router);
        m_common.install(router);
    }

private:
    WebhooksRuntime m_runtime; // SINGLE runtime instance
    WebhooksInputSupport m_input_support;
    ::app::webhook::policy::WebhooksPolicy m_policy;
    RequestSupport m_request_support;
    WebhooksHandlersV1 m_v1;
    WebhooksHandlersV2 m_v2;
    WebhooksHandlersCommon m_common;
};

} // namespace app::handlers::webhooks
