export module blog_operations:members_create_flow;

import std;
import core_workflow;
import :members_create_step;

export namespace blog::handlers::operations::members::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_create").step<ExecuteStep>().build();
    }
};

} // namespace
