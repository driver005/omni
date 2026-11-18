export module blog_operations:members_edit_flow;

import std;
import core_workflow;
import :members_edit_step;

export namespace blog::handlers::operations::members::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_edit").step<ExecuteStep>().build();
    }
};

} // namespace
