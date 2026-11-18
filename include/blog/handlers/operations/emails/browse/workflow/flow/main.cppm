export module blog_operations:emails_browse_flow;

import std;
import core_workflow;
import :emails_browse_step;

export namespace blog::handlers::operations::emails::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("emails_browse").step<ExecuteStep>().build();
    }
};

} // namespace
