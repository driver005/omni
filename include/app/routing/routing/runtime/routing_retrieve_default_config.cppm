export module app_routing:routing_runtime_routing_retrieve_default_config;

import std;
import core_error;
import core_serde;
import app_context;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingRetrieveDefaultConfigRuntime
{
public:
    explicit RoutingRetrieveDefaultConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_retrieve_default_config(const ::core::behavior::CallContext& ctx) const
    {
        const auto profile = ctx.profile_id;
        if (ctx_is_v2(ctx)) {
            if (const auto validation_error =
                    m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    *validation_error
                )};
            }
        }
        const auto connectors = m_support.default_config(profile);
        m_support.track("routing_retrieve_default_config");
        return ResponseDocument::from(connectors);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
