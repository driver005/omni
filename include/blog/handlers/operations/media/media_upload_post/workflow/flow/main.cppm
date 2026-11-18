export module blog_operations:media_media_upload_post_flow;

import std;
import core_workflow;
import :media_media_upload_post_step;

export namespace blog::handlers::operations::media::media_upload_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("media_media_upload_post").step<ExecuteStep>().build();
    }
};

} // namespace
