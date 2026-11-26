export module app_payment:refunds_handlers_v2;

import std;
import interfaces;
import core_router;
import app_context;

import :refunds_input;
import :refunds_result;
import :refunds_runtime;
import :refunds_handlers_v2_create_retrieve;
import :refunds_handlers_v2_list;
import :refunds_handlers_v2_metadata_update;

export namespace app::handlers::refunds {

// v2 facade. Per-route install calls replay the original v2/refunds install() order.
class RefundsHandlersV2
{
public:
    RefundsHandlersV2(
        ::app::context::AppContext& ctx,
        RefundsRuntime& runtime,
        RefundsInputSupport& input_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_input_support{input_support},
        m_create_retrieve{ctx, runtime, input_support},
        m_list{ctx, runtime, input_support},
        m_metadata_update{ctx, runtime, input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_create_retrieve.install_route_16(router);
        m_create_retrieve.install_route_17(router);
        m_create_retrieve.install_route_18(router);
        m_metadata_update.install_route_19(router);
        m_list.install_route_20(router);
    }

private:
    ::app::context::AppContext& m_context;
    RefundsRuntime& m_runtime;
    RefundsInputSupport& m_input_support;
    RefundsV2CreateRetrieveHandlers m_create_retrieve;
    RefundsV2ListHandlers m_list;
    RefundsV2MetadataUpdateHandlers m_metadata_update;
};

} // namespace app::handlers::refunds
