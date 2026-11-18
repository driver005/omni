export module blog_operations:integrations_browse_flow;

import std;
import core_workflow;
import :integrations_browse_step;

export namespace blog::handlers::operations::integrations::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("integrations_browse").step<ExecuteStep>().build();
    }
};

} // namespace
