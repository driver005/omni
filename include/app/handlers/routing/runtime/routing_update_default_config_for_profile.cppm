export module app_handlers_routing:runtime_routing_update_default_config_for_profile;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class RoutingUpdateDefaultConfigForProfileRuntime
{
public:
    explicit RoutingUpdateDefaultConfigForProfileRuntime(app::context::AppContext& context) noexcept
        :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RoutingUpdateDefaultConfigForProfileResult, core::error::AppError>
    routing_update_default_config_for_profile(const RoutingInput& input) const
    {
        auto profile = input.profile_id;
        if (profile.empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INVALID_REQUEST, "profile_id")
            };
        }
        if (const auto validation_error =
                m_support.profile_validation_error(input.auth_profile, profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }
        if (const auto validation_error =
                m_support.validate_default_connectors(profile, *input.connectors)) {
            return std::unexpected{*validation_error};
        }
        m_support.store_default_config(input.merchant_id, profile, *input.connectors);

        app::model::routing::ProfileDefaultRoutingConfig response;
        response.set_profile_id(m_support.wrap<app::model::routing::ProfileIdValue>(profile));
        response.set_connectors(*input.connectors);
        m_support.track("routing_update_default_config_for_profile");
        return RoutingUpdateDefaultConfigForProfileResult{ResponseDocument::from(response)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
