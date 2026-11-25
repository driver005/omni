export module blog_handlers:pages_gift_links_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :pages_gift_links_get_step;

export namespace blog::handlers::pages::gift_links_get::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("pages_gift_links_get")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::pages::gift_links_get::workflow
