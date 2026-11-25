export module blog_handlers:automations_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :automations_create_step;

export namespace blog::handlers::automations::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("automations_create")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::automations::create::workflow
