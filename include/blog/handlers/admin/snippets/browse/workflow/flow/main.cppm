export module blog_handlers:snippets_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :snippets_browse_step;

export namespace blog::handlers::snippets::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("snippets_browse")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::snippets::browse::workflow
