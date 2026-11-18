export module blog_handlers:settings_settings_stripe_connect_delete_flow;
export import core_workflow;
import std;
import core_workflow;
import :settings_settings_stripe_connect_delete_step;
export namespace blog::handlers::settings::settings_stripe_connect_delete::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("settings_settings_stripe_connect_delete").step<ExecuteStep>().build();
 }};
}
