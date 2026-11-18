export module blog_handlers:links_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :links_browse_step;
export namespace blog::handlers::links::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("links_browse").step<ExecuteStep>().build();
 }};
}
