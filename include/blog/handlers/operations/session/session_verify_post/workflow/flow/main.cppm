export module blog_operations:session_session_verify_post_flow;

import std;
import core_workflow;
import :session_session_verify_post_step;

export namespace blog::handlers::operations::session::session_verify_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("session_session_verify_post").step<ExecuteStep>().build();
    }
};

} // namespace
