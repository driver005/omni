export module blog_handlers:posts_copy_flow;
export import core_workflow;

import std;
import core_workflow;
import :posts_copy_step;

export namespace blog::handlers::posts::copy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_copy").step<ExecuteStep>().build();
    }
};

} // namespace blog::handlers::posts::copy::workflow
