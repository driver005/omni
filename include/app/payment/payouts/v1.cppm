export module app_payment:payouts_handlers_v1;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;

import :payouts_runtime;
import :payouts_input;
import :context;
import :payouts_handlers_v1_crud;
import :payouts_handlers_v1_list_filter;
import :payouts_handlers_v1_aggregates;
import :payouts_handlers_v1_manual_update;

export namespace app::handlers::payouts {

class PayoutsHandlersV1
{
public:
    void bind_request_support(PayoutsRequestSupport& support) noexcept
    {
        m_request_support = &support;
        m_crud.bind_request_support(support);
        m_list_filter.bind_request_support(support);
        m_aggregates.bind_request_support(support);
        m_manual_update.bind_request_support(support);
    }

    PayoutsRequestSupport* m_request_support{nullptr};

    explicit PayoutsHandlersV1(PayoutsRuntime& runtime, PayoutsInputSupport& input_support) noexcept
        :
        m_crud{runtime, input_support},
        m_list_filter{runtime, input_support},
        m_aggregates{runtime, input_support},
        m_manual_update{runtime, input_support}
    {
    }

    [[nodiscard]] utils::openapi::ApiRouter
    install_crud(utils::openapi::ApiRouter chain, core::router::RouterContext<>& router)
    {
        return m_crud.install(std::move(chain), router);
    }

    [[nodiscard]] utils::openapi::ApiRouter
    install_list_filter(utils::openapi::ApiRouter chain, core::router::RouterContext<>& router)
    {
        return m_list_filter.install(std::move(chain), router);
    }

    [[nodiscard]] utils::openapi::ApiRouter
    install_aggregates(utils::openapi::ApiRouter chain, core::router::RouterContext<>& router)
    {
        return m_aggregates.install(std::move(chain), router);
    }

    [[nodiscard]] utils::openapi::ApiRouter
    install_manual_update(utils::openapi::ApiRouter chain, core::router::RouterContext<>& router)
    {
        return m_manual_update.install(std::move(chain), router);
    }

private:
    PayoutsCrudHandlersV1 m_crud;
    PayoutsListFilterHandlersV1 m_list_filter;
    PayoutsAggregatesHandlersV1 m_aggregates;
    PayoutsManualUpdateHandlersV1 m_manual_update;
};

} // namespace app::handlers::payouts
