export module blog_handlers:users_edit_flow;
export import core_workflow;
import std;
import core_workflow;
import :users_edit_step;

export namespace blog::handlers::users::edit::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("users_edit").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::users::edit::workflow
