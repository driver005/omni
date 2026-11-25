export module app_handlers_routing:runtime_create_success_based_routing;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class CreateSuccessBasedRoutingRuntime
{
public:
    explicit CreateSuccessBasedRoutingRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CreateSuccessBasedRoutingResult, core::error::AppError>
    create_success_based_routing(const RoutingInput& input) const
    {
        auto result = m_support.dynamic_create(
            input.merchant_id,
            input.profile_id,
            input.auth_profile,
            input.body,
            input.enable,
            "create_success_based_routing",
            "Success rate based dynamic routing algorithm",
            "success_rate_based",
            "Success rate based routing"
        );
        if (!result) {
            return std::unexpected{result.error()};
        }
        return CreateSuccessBasedRoutingResult{std::move(*result)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
