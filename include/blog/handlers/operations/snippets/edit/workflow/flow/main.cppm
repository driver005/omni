export module blog_operations:snippets_edit_flow;

import std;
import core_workflow;
import :snippets_edit_step;

export namespace blog::handlers::operations::snippets::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("snippets_edit").step<ExecuteStep>().build();
    }
};

} // namespace
