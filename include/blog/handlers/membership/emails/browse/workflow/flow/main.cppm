export module blog_handlers:emails_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :emails_browse_step;
export namespace blog::handlers::emails::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("emails_browse").step<ExecuteStep>().build();
 }};
}
