export module app_handlers_payouts:handler;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :runtime;
import :input;
import :result;
import :handlers_v1;
import :handlers_v2;

export namespace app::handlers::payouts {

// Facade: one shared PayoutsRuntime; version partitions hold references to it.
class PayoutsRouteHandlers
{
public:
    explicit PayoutsRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_v1{m_runtime, m_input_support},
        m_v2{m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Router = utils::openapi::ApiRouter;
        // One "payouts" node, same registration order as the original fluent chain.
        Router chain{router, "payouts"};
        chain = m_v1.install_crud(std::move(chain), router);
        chain = m_v1.install_list_filter(std::move(chain), router);
        chain = m_v2.install(std::move(chain), router);
        chain = m_v1.install_aggregates(std::move(chain), router);
        chain = m_v1.install_manual_update(std::move(chain), router);
    }

private:
    PayoutsRuntime m_runtime;            // single shared runtime instance
    PayoutsInputSupport m_input_support; // single shared stateless helper
    PayoutsHandlersV1 m_v1;
    PayoutsHandlersV2 m_v2;
};

} // namespace app::handlers::payouts
