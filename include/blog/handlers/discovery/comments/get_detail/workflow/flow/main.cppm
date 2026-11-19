export module blog_handlers:comments_get_detail_flow;
export import core_workflow;
import std;
import core_workflow;
import :comments_get_detail_step;
export namespace blog::handlers::comments::get_detail::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("comments_get_detail").step<ExecuteStep>().build();
 }};
}
