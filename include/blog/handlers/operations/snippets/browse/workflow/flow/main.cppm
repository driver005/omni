export module blog_operations:snippets_browse_flow;

import std;
import core_workflow;
import :snippets_browse_step;

export namespace blog::handlers::operations::snippets::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("snippets_browse").step<ExecuteStep>().build();
    }
};

} // namespace
