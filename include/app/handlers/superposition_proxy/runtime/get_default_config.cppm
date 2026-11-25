export module app_handlers_superposition_proxy:runtime_get_default_config;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::superposition_proxy {

class GetDefaultConfigRuntime
{
public:
    explicit GetDefaultConfigRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetDefaultConfigResult, core::error::AppError>
    get_default_config(const GetDefaultConfigHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionGetDefaultConfig",
            "superposition.get_default_config"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return GetDefaultConfigResult{std::move(*wire)};
    }

private:
    app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
