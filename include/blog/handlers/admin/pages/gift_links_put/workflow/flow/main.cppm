export module blog_handlers:pages_gift_links_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :pages_gift_links_put_step;

export namespace blog::handlers::pages::gift_links_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("pages_gift_links_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::pages::gift_links_put::workflow
