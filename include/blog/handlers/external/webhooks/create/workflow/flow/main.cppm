export module blog_handlers:webhooks_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :webhooks_create_step;

export namespace blog::handlers::webhooks::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("webhooks_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::webhooks::create::workflow
