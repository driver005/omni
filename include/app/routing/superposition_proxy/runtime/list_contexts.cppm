export module app_routing:superposition_proxy_runtime_list_contexts;

import std;
import core_error;
import app_context;

import :superposition_proxy_input;
import :superposition_proxy_result;
import :superposition_proxy_runtime_common;

export namespace app::handlers::superposition_proxy {

class ListContextsRuntime
{
public:
    explicit ListContextsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListContextsResult, core::error::AppError>
    list_contexts(const ListContextsHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionListContexts",
            "superposition.list_contexts"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return ListContextsResult{std::move(*wire)};
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
