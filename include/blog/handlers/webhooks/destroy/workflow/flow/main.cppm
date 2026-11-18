export module blog_handlers:webhooks_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :webhooks_destroy_step;
export namespace blog::handlers::webhooks::destroy::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("webhooks_destroy").step<ExecuteStep>().build();
 }};
}
