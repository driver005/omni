export module blog_handlers:authors_read_by_id_flow;
export import core_workflow;
import std;
import core_workflow;
import :authors_read_by_id_step;
export namespace blog::handlers::authors::read_by_id::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("authors_read_by_id").step<ExecuteStep>().build();
 }};
}
