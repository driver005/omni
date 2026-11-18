export module blog_operations:pages_gift_links_post_flow;

import std;
import core_workflow;
import :pages_gift_links_post_step;

export namespace blog::handlers::operations::pages::gift_links_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("pages_gift_links_post").step<ExecuteStep>().build();
    }
};

} // namespace
