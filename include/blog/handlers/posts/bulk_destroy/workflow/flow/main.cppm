export module blog_handlers:posts_bulk_destroy_flow;

import std;
import core_workflow;
import :posts_bulk_destroy_step;

export namespace blog::handlers::posts::bulk_destroy::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_bulk_destroy").step<ExecuteStep>().build();
    }
};

} // namespace
