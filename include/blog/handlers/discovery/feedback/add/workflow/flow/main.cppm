export module blog_handlers:feedback_add_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :feedback_add_step;
export namespace blog::handlers::feedback::add::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("feedback_add").step<ExecuteStep>().build();
 }};
}
