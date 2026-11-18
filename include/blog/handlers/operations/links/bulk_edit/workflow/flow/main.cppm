export module blog_operations:links_bulk_edit_flow;

import std;
import core_workflow;
import :links_bulk_edit_step;

export namespace blog::handlers::operations::links::bulk_edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("links_bulk_edit").step<ExecuteStep>().build();
    }
};

} // namespace
