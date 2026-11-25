export module blog_handlers:session_create_flow;
export import core_workflow;
import std;
import core_workflow;
import :session_create_step;

export namespace blog::handlers::session::create::workflow {
class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("session_create").step<ExecuteStep>().build();
    }
};
} // namespace blog::handlers::session::create::workflow
