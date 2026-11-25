export module blog_handlers:authentication_authentication_password_reset_post_flow;
export import core_workflow;
import std;
import core_workflow;
import :authentication_authentication_password_reset_post_step;

export namespace blog::handlers::authentication::authentication_password_reset_post::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("authentication_authentication_password_reset_post")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::authentication::authentication_password_reset_post::workflow
