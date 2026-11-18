export module blog_operations:upgrades_read_by_id_flow;

import std;
import core_workflow;
import :upgrades_read_by_id_step;

export namespace blog::handlers::operations::upgrades::read_by_id::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("upgrades_read_by_id").step<ExecuteStep>().build();
    }
};

} // namespace
