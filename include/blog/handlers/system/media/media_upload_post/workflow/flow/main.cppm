export module blog_handlers:media_media_upload_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :media_media_upload_post_step;
export namespace blog::handlers::media::media_upload_post::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("media_media_upload_post").step<ExecuteStep>().build();
 }};
}
