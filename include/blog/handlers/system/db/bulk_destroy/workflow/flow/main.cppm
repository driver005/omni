export module blog_handlers:db_bulk_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :db_bulk_destroy_step;
export namespace blog::handlers::db::bulk_destroy::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("db_bulk_destroy").step<ExecuteStep>().build();
 }};
}
