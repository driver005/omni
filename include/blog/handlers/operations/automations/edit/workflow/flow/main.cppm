export module blog_operations:automations_edit_flow;

import std;
import core_workflow;
import :automations_edit_step;

export namespace blog::handlers::operations::automations::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("automations_edit").step<ExecuteStep>().build();
    }
};

} // namespace
