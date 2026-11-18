export module blog_operations:notifications_destroy_flow;

import std;
import core_workflow;
import :notifications_destroy_step;

export namespace blog::handlers::operations::notifications::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("notifications_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
