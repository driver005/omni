export module blog_handlers:emails_analytics_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :emails_analytics_put_step;
export namespace blog::handlers::emails::analytics_put::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("emails_analytics_put").step<ExecuteStep>().build();
 }};
}
