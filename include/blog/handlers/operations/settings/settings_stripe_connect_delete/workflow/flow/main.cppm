export module blog_operations:settings_settings_stripe_connect_delete_flow;

import std;
import core_workflow;
import :settings_settings_stripe_connect_delete_step;

export namespace blog::handlers::operations::settings::settings_stripe_connect_delete::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("settings_settings_stripe_connect_delete").step<ExecuteStep>().build();
    }
};

} // namespace
