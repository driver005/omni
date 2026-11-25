export module blog_handlers:members_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :members_browse_step;

export namespace blog::handlers::members::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::members::browse::workflow
