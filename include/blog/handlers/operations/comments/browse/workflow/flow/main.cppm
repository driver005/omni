export module blog_operations:comments_browse_flow;

import std;
import core_workflow;
import :comments_browse_step;

export namespace blog::handlers::operations::comments::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("comments_browse").step<ExecuteStep>().build();
    }
};

} // namespace
