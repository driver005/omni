export module blog_handlers:schedules_put_detail_flow;
export import core_workflow;
import std;
import core_workflow;
import :schedules_put_detail_step;
export namespace blog::handlers::schedules::put_detail::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("schedules_put_detail").step<ExecuteStep>().build();
 }};
}
