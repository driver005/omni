export module blog_handlers:posts_gift_links_browse_flow;
export import core_workflow;

import std;
import core_workflow;
import :posts_gift_links_browse_step;

export namespace blog::handlers::posts::gift_links_browse::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("posts_gift_links_browse")
            .step<ExecuteStep>()
            .build();
    }
};

} // namespace blog::handlers::posts::gift_links_browse::workflow
