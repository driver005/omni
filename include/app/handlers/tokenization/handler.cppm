export module app_handlers_tokenization:handler;

import std;
import core_router;
import app_context;

import :input;
import :result;
import :runtime;
import :handlers_v2;

export namespace app::handlers::tokenization {

/**
 * Handler of handlers: owns the single shared runtime, then delegates route
 * registration to the version partitions. Public shape is unchanged (class
 * name, ctor, install), so all.cppm / routes.cppm need no edits.
 */
class TokenizationRouteHandlers
{
public:
    explicit TokenizationRouteHandlers(app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_v2{context, m_runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    TokenizationRuntime m_runtime; // SINGLE runtime instance — shared by every family.
    TokenizationHandlersV2 m_v2;   // holds AppContext&, TokenizationRuntime&.
};

} // namespace app::handlers::tokenization
