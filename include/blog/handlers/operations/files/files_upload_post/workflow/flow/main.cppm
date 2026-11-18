export module blog_operations:files_files_upload_post_flow;

import std;
import core_workflow;
import :files_files_upload_post_step;

export namespace blog::handlers::operations::files::files_upload_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("files_files_upload_post").step<ExecuteStep>().build();
    }
};

} // namespace
