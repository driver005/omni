export module app_handlers_webhooks:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_common;
import :handlers_v1;
import :handlers_v2;

export namespace app::handlers::webhooks {

/** @brief Facade ("handler of handlers").
 *
 *  Owns the single shared WebhooksRuntime and WebhooksInputSupport; version
 *  partitions only hold references to them. No business logic lives here.
 */
class WebhooksRouteHandlers
{
public:
    explicit WebhooksRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support},
        m_common{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
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
    WebhooksHandlersV1 m_v1;
    WebhooksHandlersV2 m_v2;
    WebhooksHandlersCommon m_common;
};

} // namespace app::handlers::webhooks
