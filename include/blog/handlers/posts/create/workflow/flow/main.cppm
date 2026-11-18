export module blog_handlers:posts_create_flow;
export import core_workflow;

import std;
import core_workflow;
import :posts_create_step;

export namespace blog::handlers::posts::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_create").step<ExecuteStep>().build();
    }
};

} // namespace
