export module blog_handlers:pages_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :pages_create_step;
export namespace blog::handlers::pages::create::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("pages_create").step<ExecuteStep>().build();
 }};
}
