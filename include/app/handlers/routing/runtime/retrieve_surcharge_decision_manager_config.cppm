export module app_handlers_routing:runtime_retrieve_surcharge_decision_manager_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_dto;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class RetrieveSurchargeDecisionManagerConfigRuntime
{
public:
    explicit RetrieveSurchargeDecisionManagerConfigRuntime(
        app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<RetrieveSurchargeDecisionManagerConfigResult, core::error::AppError>
    retrieve_surcharge_decision_manager_config(const RoutingInput& input) const
    {
        const auto profile = input.profile_id;
        const auto key = m_support.decision_key(profile, "surcharge");
        const auto stored = m_support.find_config(key);
        if (!stored) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "surcharge conditional config"
            )};
        }
        auto parsed = serde::Ser::deserialize<
            app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord>(
            "application/json",
            stored->get_config_json()
        );
        if (!parsed) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INTERNAL,
                "The Surcharge Decision Config Record was not found"
            )};
        }
        m_support.track("retrieve_surcharge_decision_manager_config");
        return RetrieveSurchargeDecisionManagerConfigResult{ResponseDocument::from(*parsed)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
