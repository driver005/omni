export module blog_handlers:recommendations_recommendations_check_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :recommendations_recommendations_check_post_step;
export namespace blog::handlers::recommendations::recommendations_check_post::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("recommendations_recommendations_check_post").step<ExecuteStep>().build();
 }};
}
