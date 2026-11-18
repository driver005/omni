export module blog_operations:stats_subscriptions_get_flow;

import std;
import core_workflow;
import :stats_subscriptions_get_step;

export namespace blog::handlers::operations::stats::subscriptions_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("stats_subscriptions_get").step<ExecuteStep>().build();
    }
};

} // namespace
