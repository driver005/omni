export module blog_operations:members_signin_urls_flow;

import std;
import core_workflow;
import :members_signin_urls_step;

export namespace blog::handlers::operations::members::signin_urls::workflow {

class MainFlow
{
public:
    [[nodiscard]] core::workflow::WorkflowDefinition build() const
    {
        return core::workflow::WorkflowBuilder{}.name("members_signin_urls").step<ExecuteStep>().build();
    }
};

} // namespace
