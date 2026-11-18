export module blog_operations:comments_comment_reports_flow;

import std;
import core_workflow;
import :comments_comment_reports_step;

export namespace blog::handlers::operations::comments::comment_reports::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("comments_comment_reports").step<ExecuteStep>().build();
    }
};

} // namespace
