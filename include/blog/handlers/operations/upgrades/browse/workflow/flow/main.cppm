export module blog_operations:upgrades_browse_flow;

import std;
import core_workflow;
import :upgrades_browse_step;

export namespace blog::handlers::operations::upgrades::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("upgrades_browse").step<ExecuteStep>().build();
    }
};

} // namespace
