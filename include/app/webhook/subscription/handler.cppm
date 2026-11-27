export module app_webhook:subscription_handler;

import std;
import interfaces;
import core_router;
import core_behavior;
import app_context;

import :subscription_input;
import :webhook_policy;
import :subscription_runtime;
import :subscription_handlers_lifecycle;
import :subscription_handlers_estimate_items;
import :subscription_handlers_list;
import :subscription_handlers_update;

export namespace app::handlers::subscription {

// Facade ("handler of handlers"). Owns the single shared SubscriptionRuntime;
// each family child holds a reference to it, never its own runtime instance.
class SubscriptionRouteHandlers
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::SubscriptionPolicy>;

    explicit SubscriptionRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_policy{},
        m_request_support{m_policy},
        m_lifecycle{context, m_runtime, m_request_support},
        m_estimate_items{context, m_runtime, m_request_support},
        m_list{context, m_runtime, m_request_support},
        m_update{context, m_runtime, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_lifecycle.install(router);
        m_estimate_items.install(router);
        m_list.install(router);
        m_update.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntime m_runtime; // SINGLE runtime — shared state not duplicated
    ::app::webhook::policy::SubscriptionPolicy m_policy;
    RequestSupport m_request_support;
    LifecycleHandlers m_lifecycle; // holds AppContext& + SubscriptionRuntime&
    EstimateItemsHandlers m_estimate_items;
    ListHandlers m_list;
    UpdateHandlers m_update;
};

} // namespace app::handlers::subscription
