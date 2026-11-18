export module blog_operations:schedules_put_detail_flow;

import std;
import core_workflow;
import :schedules_put_detail_step;

export namespace blog::handlers::operations::schedules::put_detail::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("schedules_put_detail").step<ExecuteStep>().build();
    }
};

} // namespace
