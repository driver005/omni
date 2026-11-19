export module blog_handlers:authentication_authentication_invitation_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :authentication_authentication_invitation_get_step;
export namespace blog::handlers::authentication::authentication_invitation_get::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("authentication_authentication_invitation_get").step<ExecuteStep>().build();
 }};
}
