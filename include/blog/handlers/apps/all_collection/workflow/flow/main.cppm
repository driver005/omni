export module blog_handlers:apps_all_collection_flow;
export import core_workflow;
import std;
import core_workflow;
import :apps_all_collection_step;
export namespace blog::handlers::apps::all_collection::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("apps_all_collection").step<ExecuteStep>().build();
 }};
}
