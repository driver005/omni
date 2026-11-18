export module blog_operations:identities_browse_flow;

import std;
import core_workflow;
import :identities_browse_step;

export namespace blog::handlers::operations::identities::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("identities_browse").step<ExecuteStep>().build();
    }
};

} // namespace
