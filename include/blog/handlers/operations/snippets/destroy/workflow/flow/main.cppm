export module blog_operations:snippets_destroy_flow;

import std;
import core_workflow;
import :snippets_destroy_step;

export namespace blog::handlers::operations::snippets::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("snippets_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
