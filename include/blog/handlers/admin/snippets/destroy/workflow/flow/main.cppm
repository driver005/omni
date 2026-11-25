export module blog_handlers:snippets_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :snippets_destroy_step;

export namespace blog::handlers::snippets::destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("snippets_destroy")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::snippets::destroy::workflow
