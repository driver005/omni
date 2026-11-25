export module app_handlers_superposition_proxy:runtime_list_dimensions;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::superposition_proxy {

class ListDimensionsRuntime
{
public:
    explicit ListDimensionsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListDimensionsResult, core::error::AppError>
    list_dimensions(const ListDimensionsHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionListDimensions",
            "superposition.list_dimensions"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return ListDimensionsResult{std::move(*wire)};
    }

private:
    app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
