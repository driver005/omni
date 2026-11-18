export module blog_operations:images_images_upload_post_flow;

import std;
import core_workflow;
import :images_images_upload_post_step;

export namespace blog::handlers::operations::images::images_upload_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("images_images_upload_post").step<ExecuteStep>().build();
    }
};

} // namespace
