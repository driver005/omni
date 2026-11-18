export module blog_handlers:authentication_authentication_setup_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :authentication_authentication_setup_put_step;
export namespace blog::handlers::authentication::authentication_setup_put::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("authentication_authentication_setup_put").step<ExecuteStep>().build();
 }};
}
