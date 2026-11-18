export module blog_handlers:comments_comment_likes_flow;
export import core_workflow;
import std;
import core_workflow;
import :comments_comment_likes_step;
export namespace blog::handlers::comments::comment_likes::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("comments_comment_likes").step<ExecuteStep>().build();
 }};
}
