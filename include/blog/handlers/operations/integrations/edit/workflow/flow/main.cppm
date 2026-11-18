export module blog_operations:integrations_edit_flow;

import std;
import core_workflow;
import :integrations_edit_step;

export namespace blog::handlers::operations::integrations::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("integrations_edit").step<ExecuteStep>().build();
    }
};

} // namespace
