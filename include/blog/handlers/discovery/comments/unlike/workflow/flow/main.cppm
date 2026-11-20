export module blog_handlers:comments_unlike_flow;
export import core_workflow;
import :domain_all;
import std;
import core_workflow;
import :comments_unlike_step;
export namespace blog::handlers::comments::unlike::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("comments_unlike").step<ExecuteStep>().build();
 }};
}
