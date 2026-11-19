export module blog_handlers:members_bulk_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_bulk_destroy_step;
export namespace blog::handlers::members::bulk_destroy::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("members_bulk_destroy").step<ExecuteStep>().build();
 }};
}
