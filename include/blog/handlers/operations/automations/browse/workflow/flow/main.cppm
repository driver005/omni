export module blog_operations:automations_browse_flow;

import std;
import core_workflow;
import :automations_browse_step;

export namespace blog::handlers::operations::automations::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("automations_browse").step<ExecuteStep>().build();
    }
};

} // namespace
