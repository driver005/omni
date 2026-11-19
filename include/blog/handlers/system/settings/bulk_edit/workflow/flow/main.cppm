export module blog_handlers:settings_bulk_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :settings_bulk_edit_step;
export namespace blog::handlers::settings::bulk_edit::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("settings_bulk_edit").step<ExecuteStep>().build();
 }};
}
