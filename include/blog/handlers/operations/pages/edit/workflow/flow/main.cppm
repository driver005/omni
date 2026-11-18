export module blog_operations:pages_edit_flow;

import std;
import core_workflow;
import :pages_edit_step;

export namespace blog::handlers::operations::pages::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("pages_edit").step<ExecuteStep>().build();
    }
};

} // namespace
