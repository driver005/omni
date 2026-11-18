export module blog_operations:tiers_edit_flow;

import std;
import core_workflow;
import :tiers_edit_step;

export namespace blog::handlers::operations::tiers::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tiers_edit").step<ExecuteStep>().build();
    }
};

} // namespace
