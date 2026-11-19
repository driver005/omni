export module blog_handlers:automations_get_detail_flow;
export import core_workflow;
import std;
import core_workflow;
import :automations_get_detail_step;
export namespace blog::handlers::automations::get_detail::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("automations_get_detail").step<ExecuteStep>().build();
 }};
}
