export module blog_operations:newsletters_browse_flow;

import std;
import core_workflow;
import :newsletters_browse_step;

export namespace blog::handlers::operations::newsletters::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("newsletters_browse").step<ExecuteStep>().build();
    }
};

} // namespace
