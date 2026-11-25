export module app_handlers_three_ds_decision_rule:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_execute_decision_rule;

export namespace app::handlers::three_ds_decision_rule {

class ThreeDsDecisionRuleRuntime
{
public:
    explicit ThreeDsDecisionRuleRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_execute_decision_rule{context}
    {
    }

    [[nodiscard]] std::expected<ExecuteDecisionRuleResult, core::error::AppError>
    execute_decision_rule(const ExecuteDecisionRuleInput& input) const
    {
        return m_execute_decision_rule.execute_decision_rule(input);
    }

private:
    app::context::AppContext& m_context;
    ExecuteDecisionRuleRuntime m_execute_decision_rule;
};

} // namespace app::handlers::three_ds_decision_rule
