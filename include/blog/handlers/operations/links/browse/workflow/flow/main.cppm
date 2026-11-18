export module blog_operations:links_browse_flow;

import std;
import core_workflow;
import :links_browse_step;

export namespace blog::handlers::operations::links::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("links_browse").step<ExecuteStep>().build();
    }
};

} // namespace
