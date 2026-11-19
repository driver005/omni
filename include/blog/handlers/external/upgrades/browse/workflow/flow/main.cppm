export module blog_handlers:upgrades_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :upgrades_browse_step;
export namespace blog::handlers::upgrades::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("upgrades_browse").step<ExecuteStep>().build();
 }};
}
