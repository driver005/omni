export module blog_handlers:posts_edit_flow;
export import core_workflow;

import std;
import core_workflow;
import :posts_edit_step;

export namespace blog::handlers::posts::edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_edit").step<ExecuteStep>().build();
    }
};

} // namespace
