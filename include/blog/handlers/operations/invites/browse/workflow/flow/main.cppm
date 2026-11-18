export module blog_operations:invites_browse_flow;

import std;
import core_workflow;
import :invites_browse_step;

export namespace blog::handlers::operations::invites::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("invites_browse").step<ExecuteStep>().build();
    }
};

} // namespace
