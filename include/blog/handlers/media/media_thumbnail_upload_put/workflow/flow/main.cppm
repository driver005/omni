export module blog_handlers:media_media_thumbnail_upload_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :media_media_thumbnail_upload_put_step;
export namespace blog::handlers::media::media_thumbnail_upload_put::workflow {
class MainFlow { public:
 [[nodiscard]] core::workflow::WorkflowDefinition build() const {
  return core::workflow::WorkflowBuilder{}.name("media_media_thumbnail_upload_put").step<ExecuteStep>().build();
 }};
}
