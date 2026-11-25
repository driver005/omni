export module blog_handlers:automations_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :automations_edit_step;

export namespace blog::handlers::automations::edit::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("automations_edit")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::automations::edit::workflow
