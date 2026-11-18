export module blog_operations:recommendations_destroy_flow;

import std;
import core_workflow;
import :recommendations_destroy_step;

export namespace blog::handlers::operations::recommendations::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("recommendations_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
