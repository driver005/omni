export module blog_operations:automations_get_detail_flow;

import std;
import core_workflow;
import :automations_get_detail_step;

export namespace blog::handlers::operations::automations::get_detail::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("automations_get_detail").step<ExecuteStep>().build();
    }
};

} // namespace
