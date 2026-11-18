export module blog_operations:authors_browse_flow;

import std;
import core_workflow;
import :authors_browse_step;

export namespace blog::handlers::operations::authors::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("authors_browse").step<ExecuteStep>().build();
    }
};

} // namespace
