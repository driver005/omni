export module blog_handlers:snippets_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :snippets_edit_step;
export namespace blog::handlers::snippets::edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("snippets_edit").step<ExecuteStep>().build();
 }};
}
