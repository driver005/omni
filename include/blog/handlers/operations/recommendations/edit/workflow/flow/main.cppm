export module blog_operations:recommendations_edit_flow;

import std;
import core_workflow;
import :recommendations_edit_step;

export namespace blog::handlers::operations::recommendations::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("recommendations_edit").step<ExecuteStep>().build();
    }
};

} // namespace
