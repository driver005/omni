export module blog_handlers:stats_stats_posts_visitor_counts_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :stats_stats_posts_visitor_counts_post_step;
export namespace blog::handlers::stats::stats_posts_visitor_counts_post::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("stats_posts_visitor_counts_post").step<ExecuteStep>().build();
 }};
}
