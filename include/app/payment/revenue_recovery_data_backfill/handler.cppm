export module app_payment:revenue_recovery_data_backfill_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :revenue_recovery_data_backfill_input;
import :context;
import :revenue_recovery_data_backfill_result;
import :revenue_recovery_data_backfill_runtime;
import :revenue_recovery_data_backfill_handlers_v2;

export namespace app::handlers::revenue_recovery_data_backfill {

/**
 * Facade ("handler of handlers"). Owns the ONE shared runtime and input helper;
 * the version partition holds references to both and only adds its own routes.
 * Every registration in this domain is v2 (`v2/recovery/data-backfill`), so no
 * v1 partition exists. Public class name, ctor `(AppContext&)` and `install` are
 * unchanged; `all.cppm` / `routes.cppm` need no edits.
 */
class RevenueRecoveryDataBackfillRouteHandlers
{
public:
    explicit RevenueRecoveryDataBackfillRouteHandlers(::app::context::AppContext& context) noexcept
        :
        m_context{context},
        m_runtime{context},
        m_input_support{context},
        m_v2{context, m_runtime, m_input_support},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_v2.bind_request_support(m_request_support);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryDataBackfillRuntime m_runtime; // SINGLE runtime — no duplicated mutable state.
    RevenueRecoveryDataBackfillInputSupport m_input_support; // SINGLE input helper.
    RevenueRecoveryDataBackfillHandlersV2 m_v2; // holds AppContext& + runtime& + input support&.
    RevenueRecoveryDataBackfillPolicy m_policy;
    RevenueRecoveryDataBackfillRequestSupport m_request_support;
};

} // namespace app::handlers::revenue_recovery_data_backfill
