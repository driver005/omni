export module app_routing:routing_runtime_delete_decision_manager_config;

import std;
import core_serde;
import core_error;
import app_context;

import :routing_input;
import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class DeleteDecisionManagerConfigRuntime
{
public:
    explicit DeleteDecisionManagerConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    delete_decision_manager_config(const RoutingInput& input) const
    {
        const auto profile = input.profile_id;
        const auto key = m_support.decision_key(profile, "decision");
        m_support.remove_config(key);
        m_support.set_active_algo_ref(profile, "config_algo_id", std::string{});
        m_support.track("delete_decision_manager_config");
        return ResponseDocument::empty(); // StatusOk document
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
