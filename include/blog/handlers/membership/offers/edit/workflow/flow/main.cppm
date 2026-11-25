export module blog_handlers:offers_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :offers_edit_step;

export namespace blog::handlers::offers::edit::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("offers_edit").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::offers::edit::workflow
