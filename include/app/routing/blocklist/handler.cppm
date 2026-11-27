export module app_routing:blocklist_handler;

import std;
import interfaces;
import core_router;
import app_context;

import :blocklist_input;
import :blocklist_result;
import :blocklist_runtime;
import :blocklist_handlers_entries;
import :blocklist_handlers_batch;
import :blocklist_handlers_export;
import :blocklist_handlers_guard;
import :blocklist_handlers_lookup_count;
import :blocklist_handlers_clone;

export namespace app::handlers::blocklist {

// Facade ("handler of handlers"). Owns the single shared BlocklistRuntime; each
// family child holds a reference to it, never its own runtime instance.
class BlocklistRouteHandlers
{
public:
    explicit BlocklistRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy},
        m_entries{context, m_runtime, m_input_support, m_request_support},
        m_guard{context, m_runtime, m_input_support, m_request_support},
        m_lookup_count{context, m_runtime, m_input_support, m_request_support},
        m_batch{context, m_runtime, m_input_support, m_request_support},
        m_export{context, m_runtime, m_input_support, m_request_support},
        m_clone{context, m_runtime, m_input_support, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        // Deterministic in-process substitute for the compile-time
        // `all(feature = "olap", feature = "v1")` gate at routes/app.rs:2053.
        if (const auto enabled = m_context.config_store().get_value("route_features", "blocklist");
            enabled.has_value() && *enabled == "false") {
            return;
        }

        m_entries.install(router);
        m_guard.install(router);
        m_lookup_count.install(router);
        m_batch.install(router);
        m_export.install(router);
        m_clone.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    BlocklistRuntime m_runtime;
    BlocklistInputSupport m_input_support;     // SINGLE parse/auth helper.
    BlocklistPolicy m_policy;                  // Domain auth/scope policy.
    BlocklistRequestSupport m_request_support; // Shared by all family handlers.
    EntriesHandlers m_entries;
    GuardHandlers m_guard;
    LookupCountHandlers m_lookup_count;
    BatchHandlers m_batch;
    ExportHandlers m_export;
    CloneHandlers m_clone;
};

} // namespace app::handlers::blocklist
