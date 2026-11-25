export module app_handlers_routing:runtime_get_dynamic_routing_volume_split;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class GetDynamicRoutingVolumeSplitRuntime
{
public:
    explicit GetDynamicRoutingVolumeSplitRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetDynamicRoutingVolumeSplitResult, core::error::AppError>
    get_dynamic_routing_volume_split(const RoutingInput& input) const
    {
        const auto profile = input.profile_id;
        if (const auto validation_error =
                m_support.profile_validation_error(input.auth_profile, profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }
        app::model::routing::RoutingVolumeSplitResponse response;
        response.set_split(m_support.dynamic_volume_split(profile));
        m_support.track("get_dynamic_routing_volume_split");
        return GetDynamicRoutingVolumeSplitResult{ResponseDocument::from(response)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
