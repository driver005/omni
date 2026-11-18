export module blog_operations:labels_edit_flow;

import std;
import core_workflow;
import :labels_edit_step;

export namespace blog::handlers::operations::labels::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("labels_edit").step<ExecuteStep>().build();
    }
};

} // namespace
