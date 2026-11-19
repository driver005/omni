export module blog_handlers:members_read_by_id_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_read_by_id_step;
export namespace blog::handlers::members::read_by_id::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("members_read_by_id").step<ExecuteStep>().build();
 }};
}
