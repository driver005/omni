export module blog_operations:snippets_create_flow;

import std;
import core_workflow;
import :snippets_create_step;

export namespace blog::handlers::operations::snippets::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("snippets_create").step<ExecuteStep>().build();
    }
};

} // namespace
