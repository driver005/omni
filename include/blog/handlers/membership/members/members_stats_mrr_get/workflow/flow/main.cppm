export module blog_handlers:members_members_stats_mrr_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_members_stats_mrr_get_step;
export namespace blog::handlers::members::members_stats_mrr_get::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("members_members_stats_mrr_get").step<ExecuteStep>().build();
 }};
}
