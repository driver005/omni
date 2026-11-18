export module blog_operations:tiers_browse_flow;

import std;
import core_workflow;
import :tiers_browse_step;

export namespace blog::handlers::operations::tiers::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tiers_browse").step<ExecuteStep>().build();
    }
};

} // namespace
