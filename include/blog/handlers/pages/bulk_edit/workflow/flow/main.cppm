export module blog_handlers:pages_bulk_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :pages_bulk_edit_step;
export namespace blog::handlers::pages::bulk_edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("pages_bulk_edit").step<ExecuteStep>().build();
 }};
}
