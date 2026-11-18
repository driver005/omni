export module blog_operations:labels_create_flow;

import std;
import core_workflow;
import :labels_create_step;

export namespace blog::handlers::operations::labels::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("labels_create").step<ExecuteStep>().build();
    }
};

} // namespace
