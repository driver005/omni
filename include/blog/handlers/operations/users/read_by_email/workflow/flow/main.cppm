export module blog_operations:users_read_by_email_flow;

import std;
import core_workflow;
import :users_read_by_email_step;

export namespace blog::handlers::operations::users::read_by_email::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("users_read_by_email").step<ExecuteStep>().build();
    }
};

} // namespace
