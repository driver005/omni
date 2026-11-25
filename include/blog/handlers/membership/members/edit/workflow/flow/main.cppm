export module blog_handlers:members_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_edit_step;

export namespace blog::handlers::members::edit::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_edit").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::members::edit::workflow
