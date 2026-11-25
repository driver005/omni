export module app_handlers_blocklist:handler;

import std;
import interfaces;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_entries;
import :handlers_batch;
import :handlers_export;
import :handlers_guard;
import :handlers_lookup_count;
import :handlers_clone;

export namespace app::handlers::blocklist {

// Facade ("handler of handlers"). Owns the single shared BlocklistRuntime; each
// family child holds a reference to it, never its own runtime instance.
class BlocklistRouteHandlers
{
public:
    explicit BlocklistRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_entries{context, m_runtime},
        m_guard{context, m_runtime},
        m_lookup_count{context, m_runtime},
        m_batch{context, m_runtime},
        m_export{context, m_runtime},
        m_clone{context, m_runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
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
    app::context::AppContext& m_context;
    BlocklistRuntime m_runtime;
    EntriesHandlers m_entries;
    GuardHandlers m_guard;
    LookupCountHandlers m_lookup_count;
    BatchHandlers m_batch;
    ExportHandlers m_export;
    CloneHandlers m_clone;
};

} // namespace app::handlers::blocklist
