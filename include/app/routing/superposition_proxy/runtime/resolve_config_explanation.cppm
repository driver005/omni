export module app_routing:superposition_proxy_runtime_resolve_config_explanation;

import std;
import core_error;
import app_context;

import :superposition_proxy_input;
import :superposition_proxy_result;
import :superposition_proxy_runtime_common;

export namespace app::handlers::superposition_proxy {

class ResolveConfigExplanationRuntime
{
public:
    explicit ResolveConfigExplanationRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ResolveConfigExplanationResult, core::error::AppError>
    resolve_config_explanation(const ResolveConfigExplanationHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionResolveConfigExplanation",
            "superposition.resolve_config_explanation"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return ResolveConfigExplanationResult{std::move(*wire)};
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
