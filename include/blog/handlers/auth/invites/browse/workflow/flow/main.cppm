export module blog_handlers:invites_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :invites_browse_step;

export namespace blog::handlers::invites::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("invites_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::invites::browse::workflow
