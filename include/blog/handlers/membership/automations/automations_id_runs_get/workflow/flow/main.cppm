export module blog_handlers:automations_automations_id_runs_get_flow;
export import core_workflow;
import std;
import core_workflow;
import :automations_automations_id_runs_get_step;

export namespace blog::handlers::automations::automations_id_runs_get::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("automations_automations_id_runs_get")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::automations::automations_id_runs_get::workflow
