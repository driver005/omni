export module blog_handlers:files_files_upload_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :files_files_upload_post_step;

export namespace blog::handlers::files::files_upload_post::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("files_files_upload_post")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::files::files_upload_post::workflow
