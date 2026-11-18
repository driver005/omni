export module blog_operations:members_members_id_suppression_delete_flow;

import std;
import core_workflow;
import :members_members_id_suppression_delete_step;

export namespace blog::handlers::operations::members::members_id_suppression_delete::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_members_id_suppression_delete").step<ExecuteStep>().build();
    }
};

} // namespace
