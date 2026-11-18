export module blog_operations:tiers_read_by_id_flow;

import std;
import core_workflow;
import :tiers_read_by_id_step;

export namespace blog::handlers::operations::tiers::read_by_id::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("tiers_read_by_id").step<ExecuteStep>().build();
    }
};

} // namespace
