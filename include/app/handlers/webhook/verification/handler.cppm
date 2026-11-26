export module app_webhook:verification_handler;

import std;
import core_router;
import app_context;

import :verification_input;
import :verification_result;
import :verification_runtime;
import :verification_handlers_v1;
import :verification_handlers_v2;

export namespace app::handlers::verification {

/** @brief Facade: owns the single runtime/input support and composes the
 *  versioned handler partitions. Public surface is unchanged so callers in
 *  `all.cppm`/`routes.cppm` need no edits. */
class VerificationRouteHandlers
{
public:
    explicit VerificationRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_v1{context, m_runtime, m_input_support},
        m_v2{context, m_runtime, m_input_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_v1.install_v1(router);
        m_v2.install_v2(router);
    }

private:
    VerificationRuntime m_runtime;            // SINGLE runtime instance
    VerificationInputSupport m_input_support; // SINGLE input-support instance
    VerificationHandlersV1 m_v1;              // holds references, not its own runtime
    VerificationHandlersV2 m_v2;
};

} // namespace app::handlers::verification
