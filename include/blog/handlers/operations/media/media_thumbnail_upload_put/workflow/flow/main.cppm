export module blog_operations:media_media_thumbnail_upload_put_flow;

import std;
import core_workflow;
import :media_media_thumbnail_upload_put_step;

export namespace blog::handlers::operations::media::media_thumbnail_upload_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("media_media_thumbnail_upload_put").step<ExecuteStep>().build();
    }
};

} // namespace
