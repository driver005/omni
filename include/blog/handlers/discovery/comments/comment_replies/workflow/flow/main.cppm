export module blog_handlers:comments_comment_replies_flow;
export import core_workflow;
import std;
import core_workflow;
import :comments_comment_replies_step;
export namespace blog::handlers::comments::comment_replies::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("comments_comment_replies").step<ExecuteStep>().build();
 }};
}
