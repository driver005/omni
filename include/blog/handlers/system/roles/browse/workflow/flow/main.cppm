export module blog_handlers:roles_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :roles_browse_step;

export namespace blog::handlers::roles::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("roles_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::roles::browse::workflow
