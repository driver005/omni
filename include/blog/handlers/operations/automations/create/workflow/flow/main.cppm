export module blog_operations:automations_create_flow;

import std;
import core_workflow;
import :automations_create_step;

export namespace blog::handlers::operations::automations::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("automations_create").step<ExecuteStep>().build();
    }
};

} // namespace
