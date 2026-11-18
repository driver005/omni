export module blog_operations:posts_edit_flow;

import std;
import core_workflow;
import :posts_edit_step;

export namespace blog::handlers::operations::posts::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_edit").step<ExecuteStep>().build();
    }
};

} // namespace
