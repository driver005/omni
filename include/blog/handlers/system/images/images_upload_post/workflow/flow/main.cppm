export module blog_handlers:images_images_upload_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :images_images_upload_post_step;

export namespace blog::handlers::images::images_upload_post::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("images_images_upload_post")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::images::images_upload_post::workflow
