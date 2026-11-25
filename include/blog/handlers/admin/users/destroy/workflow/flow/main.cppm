export module blog_handlers:users_destroy_flow;
export import core_workflow;
import std;
import core_workflow;
import :users_destroy_step;

export namespace blog::handlers::users::destroy::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("users_destroy").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::users::destroy::workflow
