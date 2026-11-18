export module blog_operations:authentication_authentication_invitation_post_flow;

import std;
import core_workflow;
import :authentication_authentication_invitation_post_step;

export namespace blog::handlers::operations::authentication::authentication_invitation_post::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("authentication_authentication_invitation_post").step<ExecuteStep>().build();
    }
};

} // namespace
