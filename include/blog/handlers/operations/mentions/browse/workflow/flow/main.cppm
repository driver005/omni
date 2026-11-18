export module blog_operations:mentions_browse_flow;

import std;
import core_workflow;
import :mentions_browse_step;

export namespace blog::handlers::operations::mentions::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("mentions_browse").step<ExecuteStep>().build();
    }
};

} // namespace
