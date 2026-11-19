export module blog_handlers:tags_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :tags_edit_step;
export namespace blog::handlers::tags::edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("tags_edit").step<ExecuteStep>().build();
 }};
}
