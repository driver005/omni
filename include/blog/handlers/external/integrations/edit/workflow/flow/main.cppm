export module blog_handlers:integrations_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :integrations_edit_step;
export namespace blog::handlers::integrations::edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("integrations_edit").step<ExecuteStep>().build();
 }};
}
