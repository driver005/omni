export module blog_handlers:config_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :config_browse_step;
export namespace blog::handlers::config::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("config_browse").step<ExecuteStep>().build();
 }};
}
