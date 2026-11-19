export module blog_handlers:newsletters_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :newsletters_browse_step;
export namespace blog::handlers::newsletters::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("newsletters_browse").step<ExecuteStep>().build();
 }};
}
