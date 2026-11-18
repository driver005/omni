export module blog_handlers:recommendations_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :recommendations_browse_step;
export namespace blog::handlers::recommendations::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("recommendations_browse").step<ExecuteStep>().build();
 }};
}
