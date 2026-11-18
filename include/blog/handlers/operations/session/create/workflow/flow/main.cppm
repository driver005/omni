export module blog_operations:session_create_flow;

import std;
import core_workflow;
import :session_create_step;

export namespace blog::handlers::operations::session::create::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("session_create").step<ExecuteStep>().build();
    }
};

} // namespace
