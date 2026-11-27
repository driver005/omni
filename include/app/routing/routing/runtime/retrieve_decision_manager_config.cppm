export module app_routing:routing_runtime_retrieve_decision_manager_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_dto;

import :routing_input;
import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RetrieveDecisionManagerConfigRuntime
{
public:
    explicit RetrieveDecisionManagerConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    retrieve_decision_manager_config(const RoutingInput& input) const
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
        const auto key = m_support.decision_key(profile, "decision");
        const auto stored = m_support.find_config(key);
        if (!stored) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "conditional config")
            };
        }
        auto parsed =
            serde::Ser::deserialize<::app::dto::conditional_configs::DecisionManagerRecord>(
                "application/json",
                stored->get_config_json()
            );
        if (!parsed) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INTERNAL,
                "The Conditional Config Record was not found"
            )};
        }
        m_support.track("retrieve_decision_manager_config");
        return ResponseDocument::from(*parsed);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
