export module app_routing:routing_runtime_routing_retrieve_default_config;

import std;
import core_error;
import core_serde;
import app_context;

import :routing_input;
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

    [[nodiscard]] std::expected<RoutingRetrieveDefaultConfigResult, core::error::AppError>
    routing_retrieve_default_config(const RoutingInput& input) const
    {
        const auto profile = input.profile_id;
        if (input.is_v2) {
            if (const auto validation_error =
                    m_support.profile_validation_error(input.auth_profile, profile)) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    *validation_error
                )};
            }
        }
        const auto connectors = m_support.default_config(profile);
        m_support.track("routing_retrieve_default_config");
        return RoutingRetrieveDefaultConfigResult{ResponseDocument::from(connectors)};
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
