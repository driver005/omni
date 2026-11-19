export module blog_handlers:integrations_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :integrations_browse_step;
export namespace blog::handlers::integrations::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("integrations_browse").step<ExecuteStep>().build();
 }};
}
