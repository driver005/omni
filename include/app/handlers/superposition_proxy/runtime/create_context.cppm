export module app_handlers_superposition_proxy:runtime_create_context;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::superposition_proxy {

class CreateContextRuntime
{
public:
    explicit CreateContextRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CreateContextResult, core::error::AppError>
    create_context(const CreateContextHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionCreateContext",
            "superposition.create_context"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return CreateContextResult{std::move(*wire)};
    }

private:
    app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
