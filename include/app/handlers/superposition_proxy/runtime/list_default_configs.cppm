export module app_handlers_superposition_proxy:runtime_list_default_configs;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::superposition_proxy {

class ListDefaultConfigsRuntime
{
public:
    explicit ListDefaultConfigsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListDefaultConfigsResult, core::error::AppError>
    list_default_configs(const ListDefaultConfigsHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionListDefaultConfigs",
            "superposition.list_default_configs"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return ListDefaultConfigsResult{std::move(*wire)};
    }

private:
    app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
