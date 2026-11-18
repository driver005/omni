export module blog_operations:actions_browse_flow;

import std;
import core_workflow;
import :actions_browse_step;

export namespace blog::handlers::operations::actions::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("actions_browse").step<ExecuteStep>().build();
    }
};

} // namespace
