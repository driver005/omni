export module app_handlers_superposition_proxy:runtime_get_dimension;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::superposition_proxy {

class GetDimensionRuntime
{
public:
    explicit GetDimensionRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetDimensionResult, core::error::AppError>
    get_dimension(const GetDimensionHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionGetDimension",
            "superposition.get_dimension"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return GetDimensionResult{std::move(*wire)};
    }

private:
    app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
