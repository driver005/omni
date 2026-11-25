export module app_handlers_routing:runtime_set_dynamic_routing_volume_split;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class SetDynamicRoutingVolumeSplitRuntime
{
public:
    explicit SetDynamicRoutingVolumeSplitRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<SetDynamicRoutingVolumeSplitResult, core::error::AppError>
    set_dynamic_routing_volume_split(const RoutingInput& input) const
    {
        const auto split = input.split;
        if (!split) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INVALID_REQUEST, "split")
            };
        }
        // crate::consts::DYNAMIC_ROUTING_MAX_VOLUME = 100.
        if (*split > 100) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Dynamic routing volume split should be less than 100"
            )};
        }
        const auto profile = input.profile_id;
        if (const auto validation_error =
                m_support.profile_validation_error(input.auth_profile, profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }
        m_support.update_dynamic_volume_split(profile, *split);
        app::model::routing::RoutingVolumeSplit response;
        response.set_routing_type(app::model::routing::RoutingType::Dynamic);
        response.set_split(*split);
        m_support.track("set_dynamic_routing_volume_split");
        return SetDynamicRoutingVolumeSplitResult{ResponseDocument::from(response)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
