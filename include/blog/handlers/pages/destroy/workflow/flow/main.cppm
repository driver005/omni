export module blog_handlers:pages_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :pages_destroy_step;
export namespace blog::handlers::pages::destroy::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("pages_destroy").step<ExecuteStep>().build();
 }};
}
