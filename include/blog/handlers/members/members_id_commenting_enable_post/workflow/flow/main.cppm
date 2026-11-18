export module blog_handlers:members_members_id_commenting_enable_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_members_id_commenting_enable_post_step;
export namespace blog::handlers::members::members_id_commenting_enable_post::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("members_members_id_commenting_enable_post").step<ExecuteStep>().build();
 }};
}
