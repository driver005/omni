export module app_handlers_routing:runtime_create_elimination_routing;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class CreateEliminationRoutingRuntime
{
public:
    explicit CreateEliminationRoutingRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CreateEliminationRoutingResult, core::error::AppError>
    create_elimination_routing(const RoutingInput& input) const
    {
        auto result = m_support.dynamic_create(
            input.merchant_id,
            input.profile_id,
            input.auth_profile,
            input.body,
            input.enable,
            "create_elimination_routing",
            "Elimination based dynamic routing algorithm",
            "elimination",
            "Elimination routing"
        );
        if (!result) {
            return std::unexpected{result.error()};
        }
        return CreateEliminationRoutingResult{std::move(*result)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
