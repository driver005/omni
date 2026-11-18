export module blog_handlers:actions_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :actions_browse_step;
export namespace blog::handlers::actions::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("actions_browse").step<ExecuteStep>().build();
 }};
}
