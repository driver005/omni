export module blog_handlers:member_metafields_members_metafields_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :member_metafields_members_metafields_browse_step;
export namespace blog::handlers::member_metafields_members::metafields_browse::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("member_metafields_members_metafields_browse").step<ExecuteStep>().build();
 }};
}
