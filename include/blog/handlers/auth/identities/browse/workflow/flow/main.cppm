export module blog_handlers:identities_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :identities_browse_step;
export namespace blog::handlers::identities::browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("identities_browse").step<ExecuteStep>().build();
 }};
}
