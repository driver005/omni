export module blog_handlers:posts_read_by_slug_flow;
export import core_workflow;

import std;
import core_workflow;
import :posts_read_by_slug_step;

export namespace blog::handlers::posts::read_by_slug::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("posts_read_by_slug")
            .step<ExecuteStep>()
            .build();
    }
};

} // namespace blog::handlers::posts::read_by_slug::workflow
