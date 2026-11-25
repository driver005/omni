export module blog_handlers:users_browse_flow;
export import core_workflow;
import std;
import core_workflow;
import :users_browse_step;

export namespace blog::handlers::users::browse::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("users_browse").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::users::browse::workflow
