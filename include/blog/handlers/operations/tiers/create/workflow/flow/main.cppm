export module blog_operations:tiers_create_flow;

import std;
import core_workflow;
import :tiers_create_step;

export namespace blog::handlers::operations::tiers::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tiers_create").step<ExecuteStep>().build();
    }
};

} // namespace
