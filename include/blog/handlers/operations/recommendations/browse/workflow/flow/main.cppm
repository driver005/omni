export module blog_operations:recommendations_browse_flow;

import std;
import core_workflow;
import :recommendations_browse_step;

export namespace blog::handlers::operations::recommendations::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("recommendations_browse").step<ExecuteStep>().build();
    }
};

} // namespace
