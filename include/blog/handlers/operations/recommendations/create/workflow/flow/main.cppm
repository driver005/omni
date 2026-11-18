export module blog_operations:recommendations_create_flow;

import std;
import core_workflow;
import :recommendations_create_step;

export namespace blog::handlers::operations::recommendations::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("recommendations_create").step<ExecuteStep>().build();
    }
};

} // namespace
