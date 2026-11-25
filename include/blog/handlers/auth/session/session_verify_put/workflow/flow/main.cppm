export module blog_handlers:session_session_verify_put_flow;
export import core_workflow;
import std;
import core_workflow;
import :session_session_verify_put_step;

export namespace blog::handlers::session::session_verify_put::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}
            .name("session_session_verify_put")
            .step<ExecuteStep>()
            .build();
    }
};
} // namespace blog::handlers::session::session_verify_put::workflow
