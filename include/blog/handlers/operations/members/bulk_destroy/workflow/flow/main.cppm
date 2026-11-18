export module blog_operations:members_bulk_destroy_flow;

import std;
import core_workflow;
import :members_bulk_destroy_step;

export namespace blog::handlers::operations::members::bulk_destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_bulk_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
