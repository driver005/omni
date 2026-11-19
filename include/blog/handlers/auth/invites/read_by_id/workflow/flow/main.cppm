export module blog_handlers:invites_read_by_id_flow;
export import core_workflow;
import std;
import core_workflow;
import :invites_read_by_id_step;
export namespace blog::handlers::invites::read_by_id::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("invites_read_by_id").step<ExecuteStep>().build();
 }};
}
