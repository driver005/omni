export module blog_operations:members_bulk_edit_flow;

import std;
import core_workflow;
import :members_bulk_edit_step;

export namespace blog::handlers::operations::members::bulk_edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_bulk_edit").step<ExecuteStep>().build();
    }
};

} // namespace
