export module blog_handlers:settings_settings_routes_yaml_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :settings_settings_routes_yaml_post_step;
export namespace blog::handlers::settings::settings_routes_yaml_post::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("settings_settings_routes_yaml_post").step<ExecuteStep>().build();
 }};
}
