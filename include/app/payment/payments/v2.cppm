export module app_payment:payments_handlers_v2;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;
import :payments_input;
import :payments_result;
import :payments_runtime;
import :payments_handlers_v2_create_intent;
import :payments_handlers_v2_list;
import :payments_handlers_v2_intent;
import :payments_handlers_v2_status;
import :payments_handlers_v2_confirm;
import :payments_handlers_v2_capture_cancel;
import :payments_handlers_v2_sessions;
import :payments_handlers_v2_filters_aggregates;
import :payments_handlers_v2_misc;

export namespace app::handlers::payments {

// v2 version facade. Composes family partitions and replays the exact original
// add_route order (registry is append-only, so registration order is observable).
class PaymentsHandlersV2
{
public:
    PaymentsHandlersV2(
        ::app::context::AppContext& ctx,
        PaymentsScopeState& scope,
        PaymentsInputSupport& input_support,
        PaymentsRuntime& runtime
    ) noexcept :
        m_context{ctx},
        m_scope{scope},
        m_input_support{input_support},
        m_runtime{runtime},
        m_create_intent{ctx, scope, input_support, runtime},
        m_list{ctx, scope, input_support, runtime},
        m_intent{ctx, scope, input_support, runtime},
        m_status{ctx, scope, input_support, runtime},
        m_confirm{ctx, scope, input_support, runtime},
        m_capture_cancel{ctx, scope, input_support, runtime},
        m_sessions{ctx, scope, input_support, runtime},
        m_filters_aggregates{ctx, scope, input_support, runtime},
        m_misc{ctx, scope, input_support, runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_create_intent.install_route_00(router);
        m_filters_aggregates.install_route_01(router);
        m_filters_aggregates.install_route_02(router);
        m_create_intent.install_route_03(router);
        m_list.install_route_04(router);
        m_list.install_route_05(router);
        m_list.install_route_06(router);
        m_filters_aggregates.install_route_07(router);
        m_misc.install_route_08(router);
        m_filters_aggregates.install_route_09(router);
        m_intent.install_route_10(router);
        m_confirm.install_route_11(router);
        m_intent.install_route_12(router);
        m_intent.install_route_13(router);
        m_confirm.install_route_14(router);
        m_confirm.install_route_15(router);
        m_intent.install_route_16(router);
        m_intent.install_route_17(router);
        m_create_intent.install_route_18(router);
        m_sessions.install_route_19(router);
        m_status.install_route_20(router);
        m_status.install_route_21(router);
        m_sessions.install_route_22(router);
        m_intent.install_route_23(router);
        m_sessions.install_route_24(router);
        m_sessions.install_route_25(router);
        m_capture_cancel.install_route_26(router);
        m_capture_cancel.install_route_27(router);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
    PaymentsV2CreateIntentHandlers m_create_intent;
    PaymentsV2ListHandlers m_list;
    PaymentsV2IntentHandlers m_intent;
    PaymentsV2StatusHandlers m_status;
    PaymentsV2ConfirmHandlers m_confirm;
    PaymentsV2CaptureCancelHandlers m_capture_cancel;
    PaymentsV2SessionsHandlers m_sessions;
    PaymentsV2FiltersAggregatesHandlers m_filters_aggregates;
    PaymentsV2MiscHandlers m_misc;
};

} // namespace app::handlers::payments
