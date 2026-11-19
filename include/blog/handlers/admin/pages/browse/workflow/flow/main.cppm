export module blog_handlers:pages_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :pages_browse_step;
export namespace blog::handlers::pages::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("pages_browse").step<ExecuteStep>().build();
 }};
}
