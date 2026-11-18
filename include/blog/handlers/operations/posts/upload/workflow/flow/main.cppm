export module blog_operations:posts_upload_flow;

import std;
import core_workflow;
import :posts_upload_step;

export namespace blog::handlers::operations::posts::upload::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_upload").step<ExecuteStep>().build();
    }
};

} // namespace
