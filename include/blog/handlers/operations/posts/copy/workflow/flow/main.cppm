export module blog_operations:posts_copy_flow;

import std;
import core_workflow;
import :posts_copy_step;

export namespace blog::handlers::operations::posts::copy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_copy").step<ExecuteStep>().build();
    }
};

} // namespace
