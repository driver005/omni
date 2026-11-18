export module blog_handlers:members_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_create_step;
export namespace blog::handlers::members::create::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("members_create").step<ExecuteStep>().build();
 }};
}
