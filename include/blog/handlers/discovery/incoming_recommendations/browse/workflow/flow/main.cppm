export module blog_handlers:incoming_recommendations_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :incoming_recommendations_browse_step;
export namespace blog::handlers::incoming_recommendations::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("incoming_recommendations_browse").step<ExecuteStep>().build();
 }};
}
