export module blog_operations:members_members_stats_mrr_get_flow;

import std;
import core_workflow;
import :members_members_stats_mrr_get_step;

export namespace blog::handlers::operations::members::members_stats_mrr_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_members_stats_mrr_get").step<ExecuteStep>().build();
    }
};

} // namespace
