export module blog_operations:notifications_create_flow;

import std;
import core_workflow;
import :notifications_create_step;

export namespace blog::handlers::operations::notifications::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("notifications_create").step<ExecuteStep>().build();
    }
};

} // namespace
