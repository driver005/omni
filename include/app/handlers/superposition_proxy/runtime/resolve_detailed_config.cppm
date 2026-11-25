export module app_handlers_superposition_proxy:runtime_resolve_detailed_config;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::superposition_proxy {

class ResolveDetailedConfigRuntime
{
public:
    explicit ResolveDetailedConfigRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ResolveDetailedConfigResult, core::error::AppError>
    resolve_detailed_config(const ResolveDetailedConfigHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionResolveDetailedConfig",
            "superposition.resolve_detailed_config"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return ResolveDetailedConfigResult{std::move(*wire)};
    }

private:
    app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
