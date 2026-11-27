export module app_routing:routing_runtime_create_success_based_routing;

import std;
import core_serde;
import core_error;
import app_context;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class CreateSuccessBasedRoutingRuntime
{
public:
    explicit CreateSuccessBasedRoutingRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> create_success_based_routing(
        const std::optional<serde::Value>& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        auto result = m_support.dynamic_create(
            ctx.merchant_id,
            ctx.profile_id,
            ctx_auth_profile(ctx),
            body,
            ctx_enable(ctx),
            "create_success_based_routing",
            "Success rate based dynamic routing algorithm",
            "success_rate_based",
            "Success rate based routing"
        );
        if (!result) {
            return std::unexpected{result.error()};
        }
        return std::move(*result);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
