export module app_payment:refunds_handlers_v1;

import std;
import interfaces;
import core_router;
import app_context;

import :refunds_input;
import :context;
import :refunds_result;
import :refunds_runtime;
import :refunds_handlers_v1_create_retrieve;
import :refunds_handlers_v1_list;
import :refunds_handlers_v1_filter;
import :refunds_handlers_v1_aggregate;
import :refunds_handlers_v1_update_reverse_manual_update;

export namespace app::handlers::refunds {

// v1 facade. Per-route install calls replay the original install() order exactly:
// the route registry is append-only, so registration order is observable.
class RefundsHandlersV1
{
public:
    void bind_request_support(RefundsRequestSupport& support) noexcept
    {
        m_request_support = &support;
        m_create_retrieve.bind_request_support(support);
        m_list.bind_request_support(support);
        m_filter.bind_request_support(support);
        m_aggregate.bind_request_support(support);
        m_update_reverse_manual_update.bind_request_support(support);
    }

    RefundsRequestSupport* m_request_support{nullptr};

    RefundsHandlersV1(
        ::app::context::AppContext& ctx,
        RefundsRuntime& runtime,
        RefundsInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support},
        m_create_retrieve{ctx, runtime, input_support},
        m_list{ctx, runtime, input_support},
        m_filter{ctx, runtime, input_support},
        m_aggregate{ctx, runtime, input_support},
        m_update_reverse_manual_update{ctx, runtime, input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_create_retrieve.install_route_1(router);
        m_create_retrieve.install_route_2(router);
        m_create_retrieve.install_route_3(router);
        m_update_reverse_manual_update.install_route_4(router);
        m_update_reverse_manual_update.install_route_5(router);
        m_list.install_route_6(router);
        m_list.install_route_7(router);
        m_filter.install_route_8(router);
        m_list.install_route_9(router);
        m_filter.install_route_10(router);
        m_filter.install_route_11(router);
        m_aggregate.install_route_12(router);
        m_aggregate.install_route_13(router);
        m_filter.install_route_14(router);
        m_update_reverse_manual_update.install_route_15(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
    RefundsV1CreateRetrieveHandlers m_create_retrieve;
    RefundsV1ListHandlers m_list;
    RefundsV1FilterHandlers m_filter;
    RefundsV1AggregateHandlers m_aggregate;
    RefundsV1UpdateReverseManualUpdateHandlers m_update_reverse_manual_update;
};

} // namespace app::handlers::refunds
