export module blog_operations:stats_stats_posts_member_counts_post_flow;

import std;
import core_workflow;
import :stats_stats_posts_member_counts_post_step;

export namespace blog::handlers::operations::stats::stats_posts_member_counts_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("stats_stats_posts_member_counts_post").step<ExecuteStep>().build();
    }
};

} // namespace
