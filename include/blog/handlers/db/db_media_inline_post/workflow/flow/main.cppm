export module blog_handlers:db_db_media_inline_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :db_db_media_inline_post_step;
export namespace blog::handlers::db::db_media_inline_post::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("db_db_media_inline_post").step<ExecuteStep>().build();
 }};
}
