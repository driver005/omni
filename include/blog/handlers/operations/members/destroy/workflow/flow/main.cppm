export module blog_operations:members_destroy_flow;

import std;
import core_workflow;
import :members_destroy_step;

export namespace blog::handlers::operations::members::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
