export module blog_operations:session_session_verify_put_flow;

import std;
import core_workflow;
import :session_session_verify_put_step;

export namespace blog::handlers::operations::session::session_verify_put::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("session_session_verify_put").step<ExecuteStep>().build();
    }
};

} // namespace
