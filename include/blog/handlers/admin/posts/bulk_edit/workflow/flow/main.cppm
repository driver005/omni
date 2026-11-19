export module blog_handlers:posts_bulk_edit_flow;
export import core_workflow;

import std;
import core_workflow;
import :posts_bulk_edit_step;

export namespace blog::handlers::posts::bulk_edit::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_bulk_edit").step<ExecuteStep>().build();
    }
};

} // namespace
