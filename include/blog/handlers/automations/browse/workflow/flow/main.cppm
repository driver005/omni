export module blog_handlers:automations_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :automations_browse_step;
export namespace blog::handlers::automations::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("automations_browse").step<ExecuteStep>().build();
 }};
}
