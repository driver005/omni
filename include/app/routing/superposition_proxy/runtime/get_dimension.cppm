export module app_routing:superposition_proxy_runtime_get_dimension;

import std;
import core_serde;
import core_error;
import app_context;
import core_behavior;

import :superposition_proxy_input;
import :superposition_proxy_runtime_common;

export namespace app::handlers::superposition_proxy {

class GetDimensionRuntime
{
public:
    explicit GetDimensionRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> get_dimension(
        const GetDimensionRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        auto wire = m_support.finish(
            m_support.execute(request, ctx),
            "SuperpositionGetDimension",
            "superposition.get_dimension"
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
