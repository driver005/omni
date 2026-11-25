export module blog_handlers:gift_links_gift_links_remove_all_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :gift_links_gift_links_remove_all_put_step;

export namespace blog::handlers::gift_links::gift_links_remove_all_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("gift_links_gift_links_remove_all_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::gift_links::gift_links_remove_all_put::workflow
