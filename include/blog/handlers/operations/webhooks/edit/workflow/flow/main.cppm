export module blog_operations:webhooks_edit_flow;

import std;
import core_workflow;
import :webhooks_edit_step;

export namespace blog::handlers::operations::webhooks::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("webhooks_edit").step<ExecuteStep>().build();
    }
};

} // namespace
