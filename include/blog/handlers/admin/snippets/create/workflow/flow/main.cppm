export module blog_handlers:snippets_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :snippets_create_step;

export namespace blog::handlers::snippets::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("snippets_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::snippets::create::workflow
