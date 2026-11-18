export module blog_handlers:tiers_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :tiers_edit_step;
export namespace blog::handlers::tiers::edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("tiers_edit").step<ExecuteStep>().build();
 }};
}
