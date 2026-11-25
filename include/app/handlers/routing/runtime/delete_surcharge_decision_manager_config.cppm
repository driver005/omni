export module app_handlers_routing:runtime_delete_surcharge_decision_manager_config;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class DeleteSurchargeDecisionManagerConfigRuntime
{
public:
    explicit DeleteSurchargeDecisionManagerConfigRuntime(app::context::AppContext& context) noexcept
        :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<DeleteSurchargeDecisionManagerConfigResult, core::error::AppError>
    delete_surcharge_decision_manager_config(const RoutingInput& input) const
    {
        const auto profile = input.profile_id;
        const auto key = m_support.decision_key(profile, "surcharge");
        m_support.remove_config(key);
        m_support.set_active_algo_ref(profile, "surcharge_config_algo_id", std::string{});
        m_support.track("delete_surcharge_decision_manager_config");
        return DeleteSurchargeDecisionManagerConfigResult{
            ResponseDocument::empty()
        }; // StatusOk document
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
