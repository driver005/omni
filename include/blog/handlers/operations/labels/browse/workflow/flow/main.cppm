export module blog_operations:labels_browse_flow;

import std;
import core_workflow;
import :labels_browse_step;

export namespace blog::handlers::operations::labels::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("labels_browse").step<ExecuteStep>().build();
    }
};

} // namespace
