export module blog_handlers:labels_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :labels_edit_step;
export namespace blog::handlers::labels::edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("labels_edit").step<ExecuteStep>().build();
 }};
}
