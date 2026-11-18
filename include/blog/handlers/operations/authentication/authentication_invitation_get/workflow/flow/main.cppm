export module blog_operations:authentication_authentication_invitation_get_flow;

import std;
import core_workflow;
import :authentication_authentication_invitation_get_step;

export namespace blog::handlers::operations::authentication::authentication_invitation_get::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("authentication_authentication_invitation_get").step<ExecuteStep>().build();
    }
};

} // namespace
