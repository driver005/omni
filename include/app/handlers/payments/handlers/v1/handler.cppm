export module app_handlers_payments:handlers_v1;

import std;
import interfaces;
import core_router;
import utils_openapi;
import app_context;
import :input;
import :result;
import :runtime;
import :handlers_v1_create;
import :handlers_v1_list;
import :handlers_v1_status;
import :handlers_v1_confirm_capture;
import :handlers_v1_authorize;
import :handlers_v1_cancel;
import :handlers_v1_redirects;
import :handlers_v1_complete_redirect;
import :handlers_v1_authorize_3ds;
import :handlers_v1_sessions;
import :handlers_v1_filters;
import :handlers_v1_aggregates;
import :handlers_v1_misc;

export namespace app::handlers::payments {

// v1 version facade. Composes family partitions and replays the exact original
// add_route order (registry is append-only, so registration order is observable).
class PaymentsHandlersV1
{
public:
    PaymentsHandlersV1(
        app::context::AppContext& ctx,
        PaymentsScopeState& scope,
        PaymentsInputSupport& input_support,
        PaymentsRuntime& runtime
    ) noexcept :
        m_context{ctx},
        m_scope{scope},
        m_input_support{input_support},
        m_runtime{runtime},
        m_create{ctx, scope, input_support, runtime},
        m_list{ctx, scope, input_support, runtime},
        m_status{ctx, scope, input_support, runtime},
        m_confirm_capture{ctx, scope, input_support, runtime},
        m_authorize{ctx, scope, input_support, runtime},
        m_cancel{ctx, scope, input_support, runtime},
        m_redirects{ctx, scope, input_support, runtime},
        m_complete_redirect{ctx, scope, input_support, runtime},
        m_authorize_3ds{ctx, scope, input_support, runtime},
        m_sessions{ctx, scope, input_support, runtime},
        m_filters{ctx, scope, input_support, runtime},
        m_aggregates{ctx, scope, input_support, runtime},
        m_misc{ctx, scope, input_support, runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_list.install_route_28(router);
        m_list.install_route_29(router);
        m_list.install_route_30(router);
        m_list.install_route_31(router);
        m_list.install_route_32(router);
        m_list.install_route_33(router);
        m_filters.install_route_34(router);
        m_filters.install_route_35(router);
        m_aggregates.install_route_36(router);
        m_aggregates.install_route_37(router);
        m_aggregates.install_route_38(router);
        m_aggregates.install_route_39(router);
        m_misc.install_route_40(router);
        m_filters.install_route_41(router);
        m_create.install_route_42(router);
        m_status.install_route_43(router);
        m_status.install_route_44(router);
        m_create.install_route_45(router);
        m_create.install_route_46(router);
        m_sessions.install_route_47(router);
        m_status.install_route_48(router);
        m_status.install_route_49(router);
        m_create.install_route_50(router);
        m_sessions.install_route_51(router);
        m_confirm_capture.install_route_52(router);
        m_cancel.install_route_53(router);
        m_cancel.install_route_54(router);
        m_cancel.install_route_55(router);
        m_confirm_capture.install_route_56(router);
        m_confirm_capture.install_route_57(router);
        m_confirm_capture.install_route_58(router);
        m_authorize.install_route_59(router);
        m_authorize.install_route_60(router);
        m_authorize.install_route_61(router);
        m_authorize.install_route_62(router);
        m_authorize.install_route_63(router);
        m_authorize.install_route_64(router);
        m_status.install_route_65(router);
        m_misc.install_route_66(router);
        m_create.install_route_67(router);
        m_redirects.install_route_68(router);
        m_redirects.install_route_69(router);
        m_redirects.install_route_70(router);
        m_redirects.install_route_71(router);
        m_complete_redirect.install_route_72(router);
        m_complete_redirect.install_route_73(router);
        m_complete_redirect.install_route_74(router);
        m_complete_redirect.install_route_75(router);
        m_authorize_3ds.install_route_76(router);
        m_authorize_3ds.install_route_77(router);
        // Legacy callback resources (upstream app.rs:1196-1201);
        // callback_routes.rs:76-121,181-209). Same handlers, original tokenless
        // paths, kept for the compatibility rollout.
        m_redirects.install_route_78(router);
        m_redirects.install_route_79(router);
        m_redirects.install_route_80(router);
        m_complete_redirect.install_route_81(router);
        m_complete_redirect.install_route_82(router);
        m_authorize_3ds.install_route_83(router);
    }

private:
    app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
    PaymentsInputSupport& m_input_support;
    PaymentsRuntime& m_runtime;
    PaymentsV1CreateHandlers m_create;
    PaymentsV1ListHandlers m_list;
    PaymentsV1StatusHandlers m_status;
    PaymentsV1ConfirmCaptureHandlers m_confirm_capture;
    PaymentsV1AuthorizeHandlers m_authorize;
    PaymentsV1CancelHandlers m_cancel;
    PaymentsV1RedirectsHandlers m_redirects;
    PaymentsV1CompleteRedirectHandlers m_complete_redirect;
    PaymentsV1Authorize3dsHandlers m_authorize_3ds;
    PaymentsV1SessionsHandlers m_sessions;
    PaymentsV1FiltersHandlers m_filters;
    PaymentsV1AggregatesHandlers m_aggregates;
    PaymentsV1MiscHandlers m_misc;
};

} // namespace app::handlers::payments
