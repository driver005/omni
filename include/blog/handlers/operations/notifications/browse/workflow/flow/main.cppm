export module blog_operations:notifications_browse_flow;

import std;
import core_workflow;
import :notifications_browse_step;

export namespace blog::handlers::operations::notifications::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("notifications_browse").step<ExecuteStep>().build();
    }
};

} // namespace
