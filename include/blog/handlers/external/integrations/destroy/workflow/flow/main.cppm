export module blog_handlers:integrations_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :integrations_destroy_step;
export namespace blog::handlers::integrations::destroy::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("integrations_destroy").step<ExecuteStep>().build();
 }};
}
