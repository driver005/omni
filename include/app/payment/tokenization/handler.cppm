export module app_payment:tokenization_handler;

import std;
import core_router;
import app_context;

import :tokenization_input;
import :tokenization_result;
import :tokenization_runtime;
import :tokenization_handlers_v2;

export namespace app::handlers::tokenization {

/**
 * Handler of handlers: owns the single shared runtime, then delegates route
 * registration to the version partitions. Public shape is unchanged (class
 * name, ctor, install), so all.cppm / routes.cppm need no edits.
 */
class TokenizationRouteHandlers
{
public:
    explicit TokenizationRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_v2{context, m_runtime},
        m_input_support{context},
        m_policy{context, m_input_support},
        m_request_support{m_policy}
    {
        m_v2.bind_request_support(m_request_support);
        m_v2.bind_policy(m_policy);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v2.install(router);
    }

private:
    TokenizationRuntime m_runtime; // SINGLE runtime instance — shared by every family.
    TokenizationHandlersV2 m_v2;   // holds AppContext&, TokenizationRuntime&.
    TokenizationInputSupport m_input_support;
    TokenizationPolicy m_policy;
    TokenizationRequestSupport m_request_support;
};

} // namespace app::handlers::tokenization
