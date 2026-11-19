export module blog_handlers:users_users_id_token_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :users_users_id_token_put_step;
export namespace blog::handlers::users::users_id_token_put::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("users_users_id_token_put").step<ExecuteStep>().build();
 }};
}
