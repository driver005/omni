export module blog_handlers:posts_destroy_flow;

import std;
import core_workflow;
import :posts_destroy_step;

export namespace blog::handlers::posts::destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
