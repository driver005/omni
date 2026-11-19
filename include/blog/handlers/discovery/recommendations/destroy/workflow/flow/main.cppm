export module blog_handlers:recommendations_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :recommendations_destroy_step;
export namespace blog::handlers::recommendations::destroy::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("recommendations_destroy").step<ExecuteStep>().build();
 }};
}
