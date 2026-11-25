export module blog_handlers:webhooks_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :webhooks_edit_step;

export namespace blog::handlers::webhooks::edit::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("webhooks_edit").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::webhooks::edit::workflow
