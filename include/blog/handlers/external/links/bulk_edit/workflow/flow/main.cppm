export module blog_handlers:links_bulk_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :links_bulk_edit_step;
export namespace blog::handlers::links::bulk_edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("links_bulk_edit").step<ExecuteStep>().build();
 }};
}
