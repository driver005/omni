export module blog_operations:comments_comment_dislikes_flow;

import std;
import core_workflow;
import :comments_comment_dislikes_step;

export namespace blog::handlers::operations::comments::comment_dislikes::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("comments_comment_dislikes").step<ExecuteStep>().build();
    }
};

} // namespace
