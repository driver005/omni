export module blog_handlers:integrations_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :integrations_create_step;

export namespace blog::handlers::integrations::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("integrations_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::integrations::create::workflow
