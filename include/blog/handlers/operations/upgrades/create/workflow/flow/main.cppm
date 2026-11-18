export module blog_operations:upgrades_create_flow;

import std;
import core_workflow;
import :upgrades_create_step;

export namespace blog::handlers::operations::upgrades::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("upgrades_create").step<ExecuteStep>().build();
    }
};

} // namespace
