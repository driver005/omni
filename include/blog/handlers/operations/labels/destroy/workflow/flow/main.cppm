export module blog_operations:labels_destroy_flow;

import std;
import core_workflow;
import :labels_destroy_step;

export namespace blog::handlers::operations::labels::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("labels_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
