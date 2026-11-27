export module app_routing:superposition_proxy_runtime_resolve_detailed_config;

import std;
import core_serde;
import core_error;
import app_context;
import core_behavior;

import :superposition_proxy_input;
import :superposition_proxy_runtime_common;

export namespace app::handlers::superposition_proxy {

class ResolveDetailedConfigRuntime
{
public:
    explicit ResolveDetailedConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> resolve_detailed_config(
        const ResolveDetailedConfigRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        auto wire = m_support.finish(
            m_support.execute(request, ctx),
            "SuperpositionResolveDetailedConfig",
            "superposition.resolve_detailed_config"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return std::move(*wire);
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
