export module blog_handlers:automations_automations_poll_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :automations_automations_poll_put_step;

export namespace blog::handlers::automations::automations_poll_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("automations_automations_poll_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::automations::automations_poll_put::workflow
