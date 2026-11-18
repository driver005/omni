export module blog_operations:stats_stats_member_count_get_flow;

import std;
import core_workflow;
import :stats_stats_member_count_get_step;

export namespace blog::handlers::operations::stats::stats_member_count_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("stats_stats_member_count_get").step<ExecuteStep>().build();
    }
};

} // namespace
