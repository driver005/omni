export module app_handlers_subscription:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_lifecycle;
import :handlers_estimate_items;
import :handlers_list;
import :handlers_update;

export namespace app::handlers::subscription {

// Facade ("handler of handlers"). Owns the single shared SubscriptionRuntime;
// each family child holds a reference to it, never its own runtime instance.
class SubscriptionRouteHandlers
{
public:
    explicit SubscriptionRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_lifecycle{context, m_runtime},
        m_estimate_items{context, m_runtime},
        m_list{context, m_runtime},
        m_update{context, m_runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_lifecycle.install(router);
        m_estimate_items.install(router);
        m_list.install(router);
        m_update.install(router);
    }

private:
    app::context::AppContext& m_context;
    SubscriptionRuntime m_runtime; // SINGLE runtime — shared state not duplicated
    LifecycleHandlers m_lifecycle; // holds AppContext& + SubscriptionRuntime&
    EstimateItemsHandlers m_estimate_items;
    ListHandlers m_list;
    UpdateHandlers m_update;
};

} // namespace app::handlers::subscription
