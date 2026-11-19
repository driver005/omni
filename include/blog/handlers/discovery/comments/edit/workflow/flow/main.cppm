export module blog_handlers:comments_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :comments_edit_step;
export namespace blog::handlers::comments::edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("comments_edit").step<ExecuteStep>().build();
 }};
}
