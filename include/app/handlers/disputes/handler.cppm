export module app_handlers_disputes:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_fetch_list;
import :handlers_retrieve;
import :handlers_submit_accept;
import :handlers_misc;
import :handlers_evidence;

export namespace app::handlers::disputes {

/** @brief Port of upstream `routes/disputes.rs` + `core/disputes.rs` (SHA 7736d55). */
class DisputesRouteHandlers
{
public:
    explicit DisputesRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_fetch_list{context, m_runtime, m_input_support},
        m_retrieve{context, m_runtime, m_input_support},
        m_submit_accept{context, m_runtime, m_input_support},
        m_misc{context, m_runtime, m_input_support},
        m_evidence{context, m_runtime, m_input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        // Original registration order preserved across family fragments.
        m_fetch_list.install(router);
        m_misc.install(router);
        m_fetch_list.install_platform(router);
        m_misc.install_platform(router);
        m_submit_accept.install(router);
        m_misc.install_aggregate(router);
        m_submit_accept.install_evidence(router);
        m_evidence.install(router);
        m_retrieve.install(router);
        m_fetch_list.install_fetch(router);
    }

private:
    DisputesRuntime m_runtime;            // SINGLE runtime instance, shared by every family.
    DisputesInputSupport m_input_support; // SINGLE stateless input helper, shared by every family.
    DisputesHandlersFetchList m_fetch_list;
    DisputesHandlersRetrieve m_retrieve;
    DisputesHandlersSubmitAccept m_submit_accept;
    DisputesHandlersMisc m_misc;
    DisputesHandlersEvidence m_evidence;
};

} // namespace app::handlers::disputes
