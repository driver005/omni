export module blog_handlers:newsletters_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :newsletters_create_step;
export namespace blog::handlers::newsletters::create::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("newsletters_create").step<ExecuteStep>().build();
 }};
}
