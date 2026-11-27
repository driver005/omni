export module app_routing:routing_runtime_get_dynamic_routing_volume_split;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class GetDynamicRoutingVolumeSplitRuntime
{
public:
    explicit GetDynamicRoutingVolumeSplitRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_dynamic_routing_volume_split(const ::core::behavior::CallContext& ctx) const
    {
        const auto profile = ctx.profile_id;
        if (const auto validation_error =
                m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }
        ::app::model::routing::RoutingVolumeSplitResponse response;
        response.set_split(m_support.dynamic_volume_split(profile));
        m_support.track("get_dynamic_routing_volume_split");
        return ResponseDocument::from(response);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
