export module app_handlers_routing:runtime_delete_decision_manager_config;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class DeleteDecisionManagerConfigRuntime
{
public:
    explicit DeleteDecisionManagerConfigRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<DeleteDecisionManagerConfigResult, core::error::AppError>
    delete_decision_manager_config(const RoutingInput& input) const
    {
        const auto profile = input.profile_id;
        const auto key = m_support.decision_key(profile, "decision");
        m_support.remove_config(key);
        m_support.set_active_algo_ref(profile, "config_algo_id", std::string{});
        m_support.track("delete_decision_manager_config");
        return DeleteDecisionManagerConfigResult{ResponseDocument::empty()}; // StatusOk document
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
