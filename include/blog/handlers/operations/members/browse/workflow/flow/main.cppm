export module blog_operations:members_browse_flow;

import std;
import core_workflow;
import :members_browse_step;

export namespace blog::handlers::operations::members::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_browse").step<ExecuteStep>().build();
    }
};

} // namespace
