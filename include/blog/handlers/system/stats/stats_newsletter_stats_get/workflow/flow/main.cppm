export module blog_handlers:stats_stats_newsletter_stats_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :stats_stats_newsletter_stats_get_step;
export namespace blog::handlers::stats::stats_newsletter_stats_get::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("stats_stats_newsletter_stats_get").step<ExecuteStep>().build();
 }};
}
