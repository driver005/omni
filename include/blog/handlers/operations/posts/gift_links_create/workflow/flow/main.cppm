export module blog_operations:posts_gift_links_create_flow;

import std;
import core_workflow;
import :posts_gift_links_create_step;

export namespace blog::handlers::operations::posts::gift_links_create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("posts_gift_links_create").step<ExecuteStep>().build();
    }
};

} // namespace
