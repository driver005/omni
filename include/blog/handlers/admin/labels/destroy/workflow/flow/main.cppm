export module blog_handlers:labels_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :labels_destroy_step;
export namespace blog::handlers::labels::destroy::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("labels_destroy").step<ExecuteStep>().build();
 }};
}
