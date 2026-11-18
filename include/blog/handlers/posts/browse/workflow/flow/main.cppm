export module blog_handlers:posts_browse_flow;

import std;
import core_workflow;
import :posts_browse_step;

export namespace blog::handlers::posts::browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_browse").step<ExecuteStep>().build();
    }
};

} // namespace
