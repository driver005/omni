export module blog_handlers:users_read_by_email_flow;
export import core_workflow;
import std;
import core_workflow;
import :users_read_by_email_step;

export namespace blog::handlers::users::read_by_email::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("users_read_by_email")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::users::read_by_email::workflow
