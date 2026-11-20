export module blog_handlers:apps_app_installations_destroy_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :apps_app_installations_destroy_step;
export namespace blog::handlers::apps::app_installations_destroy::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("apps_app_installations_destroy").step<ExecuteStep>().build();
 }};
}
