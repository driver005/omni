export module blog_operations:offers_browse_flow;

import std;
import core_workflow;
import :offers_browse_step;

export namespace blog::handlers::operations::offers::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("offers_browse").step<ExecuteStep>().build();
    }
};

} // namespace
