export module blog_handlers:automations_read_by_id_flow;
export import core_workflow;
export import core_workflow;
import std;
import core_workflow;
import :automations_read_by_id_step;
export namespace blog::handlers::automations::read_by_id::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("automations_read_by_id").step<ExecuteStep>().build();
 }};
}
