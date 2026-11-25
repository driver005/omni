export module app_handlers_revenue_recovery_data_backfill:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v2;

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
        m_v2{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    app::context::AppContext& m_context;
    RevenueRecoveryDataBackfillRuntime m_runtime; // SINGLE runtime — no duplicated mutable state.
    RevenueRecoveryDataBackfillInputSupport m_input_support; // SINGLE input helper.
    RevenueRecoveryDataBackfillHandlersV2 m_v2; // holds AppContext& + runtime& + input support&.
};

} // namespace app::handlers::revenue_recovery_data_backfill
